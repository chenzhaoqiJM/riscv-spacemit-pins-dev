/**
 * pulse_gpiod_v2.cpp
 * 基于 libgpiod v2.x C++ API 的脉冲计数/转速测量程序
 *
 * 功能:
 * - 通过 GPIO 边沿事件监听编码器脉冲
 * - 计算电机转速 (RPM)
 * - 支持命令行参数输入 GPIO 号，自动计算 gpiochip 和引脚偏移
 */

#include <gpiod.hpp>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

constexpr int PINS_PER_CHIP = 32;

/* ================== EncoderSpeedMeter ================== */

class EncoderSpeedMeter {
public:
  EncoderSpeedMeter(int gpio_number, double sample_period = 0.033,
                    double encoder_ppr = 11.0, double gear_ratio = 56.0,
                    size_t queue_size = 10)
      : gpio_number_(gpio_number), sample_period_(sample_period),
        encoder_ppr_(encoder_ppr), gear_ratio_(gear_ratio),
        queue_size_(queue_size) {
    chip_index_ = gpio_number_ / PINS_PER_CHIP;
    line_offset_ = gpio_number_ % PINS_PER_CHIP;
    chip_path_ = "/dev/gpiochip" + std::to_string(chip_index_);
  }

  void start() {
    stop_flag_ = false;
    interrupt_thread_ = std::thread(&EncoderSpeedMeter::interrupt_loop, this);
    sampler_thread_ = std::thread(&EncoderSpeedMeter::sampler_loop, this);
    processor_thread_ = std::thread(&EncoderSpeedMeter::processor_loop, this);
  }

  void stop() {
    stop_flag_ = true;
    cv_.notify_all();
    if (interrupt_thread_.joinable())
      interrupt_thread_.join();
    if (sampler_thread_.joinable())
      sampler_thread_.join();
    if (processor_thread_.joinable())
      processor_thread_.join();
  }

  void print_info() const {
    std::cout << "GPIO " << gpio_number_ << " -> " << chip_path_ << " 引脚 "
              << line_offset_ << std::endl;
    std::cout << "采样周期: " << sample_period_ * 1000.0 << " ms" << std::endl;
    std::cout << "编码器 PPR: " << encoder_ppr_ << std::endl;
    std::cout << "减速比: " << gear_ratio_ << std::endl;
  }

private:
  /* ---------- GPIO ---------- */
  int gpio_number_;
  int chip_index_;
  int line_offset_;
  std::string chip_path_;

  /* ---------- 参数 ---------- */
  double sample_period_;
  double encoder_ppr_;
  double gear_ratio_;
  size_t queue_size_;

  /* ---------- 状态 ---------- */
  std::mutex lock_;
  uint64_t pulse_count_{0};
  bool has_rising_{false};

  /* ---------- 线程通信 ---------- */
  std::queue<std::pair<double, uint64_t>> queue_;
  std::atomic<bool> stop_flag_{false};
  std::condition_variable cv_;

  /* ---------- 线程 ---------- */
  std::thread interrupt_thread_;
  std::thread sampler_thread_;
  std::thread processor_thread_;

private:
  /* ================== GPIO 中断监听线程 (libgpiod v2.x) ================== */
  void interrupt_loop() {
    // 打开 GPIO 芯片
    auto chip = gpiod::chip(chip_path_);

    // 配置 line 为边沿事件监听，上拉
    auto settings = gpiod::line_settings();
    settings.set_direction(gpiod::line::direction::INPUT);
    settings.set_edge_detection(gpiod::line::edge::BOTH);
    settings.set_bias(gpiod::line::bias::PULL_UP);

    // 请求 line
    auto req_builder = chip.prepare_request();
    req_builder.set_consumer("encoder");
    req_builder.add_line_settings(line_offset_, settings);

    auto request = req_builder.do_request();

    // 创建事件缓冲区
    gpiod::edge_event_buffer event_buffer(16);

    std::cout << "开始监听 GPIO " << gpio_number_ << " (" << chip_path_
              << " 引脚 " << line_offset_ << ")..." << std::endl;

    while (!stop_flag_) {
      // 等待事件，超时 100ms
      if (!request.wait_edge_events(std::chrono::milliseconds(100)))
        continue;

      // 读取事件到缓冲区
      std::size_t num_events = request.read_edge_events(event_buffer);
      for (std::size_t i = 0; i < num_events; ++i) {
        handle_event(event_buffer.get_event(i));
      }
    }
  }

  /* ================== 中断逻辑 ================== */
  void handle_event(const gpiod::edge_event &event) {
    std::lock_guard<std::mutex> guard(lock_);

    if (event.type() == gpiod::edge_event::event_type::RISING_EDGE) {
      has_rising_ = true;
    } else if (event.type() == gpiod::edge_event::event_type::FALLING_EDGE) {
      if (has_rising_) {
        pulse_count_++;
        has_rising_ = false;
        // ★ 通知：一个完整脉冲结束了
        cv_.notify_all();
      }
    }
  }

  /* ================== 采样线程 ================== */
  void sampler_loop() {
    auto last = std::chrono::steady_clock::now();

    while (!stop_flag_) {
      std::this_thread::sleep_for(
          std::chrono::duration<double>(sample_period_));

      auto now = std::chrono::steady_clock::now();
      std::chrono::duration<double> dt = now - last;
      last = now;

      uint64_t pulse = 0;
      {
        std::unique_lock<std::mutex> lk(lock_);

        // ★ 如果正在数一个脉冲，就等它完成 (带超时避免死锁)
        cv_.wait_for(lk, std::chrono::milliseconds(50),
                     [&] { return !has_rising_ || stop_flag_.load(); });

        pulse = pulse_count_;
        pulse_count_ = 0;
      }

      if (queue_.size() < queue_size_) {
        queue_.emplace(dt.count(), pulse);
      }
    }
  }

  /* ================== 计算线程 ================== */
  void processor_loop() {
    while (!stop_flag_) {
      if (queue_.empty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        continue;
      }

      auto [dt, pulse] = queue_.front();
      queue_.pop();

      if (dt <= 0.0)
        continue;

      double encoder_turns = pulse / encoder_ppr_;
      double motor_turns = encoder_turns / gear_ratio_;
      double rps = motor_turns / dt;

      printf("脉冲数: %lu | dt: %.3f ms | 转速: %.2f RPM\n", pulse, dt * 1000.0,
             rps * 60.0);
    }
  }
};

/* ================== 全局变量和信号处理 ================== */

static EncoderSpeedMeter *g_meter = nullptr;

void sigint_handler(int) {
  if (g_meter)
    g_meter->stop();
  std::exit(0);
}

/* ================== 帮助信息 ================== */

void print_usage(const char *prog_name) {
  std::cerr << "用法: " << prog_name
            << " <gpio_number> [sample_period_ms] [encoder_ppr] [gear_ratio]"
            << std::endl;
  std::cerr << std::endl;
  std::cerr << "参数:" << std::endl;
  std::cerr << "  gpio_number      GPIO 编号 (必须)" << std::endl;
  std::cerr << "  sample_period_ms 采样周期，毫秒 (默认: 33)" << std::endl;
  std::cerr << "  encoder_ppr      编码器每转脉冲数 (默认: 11)" << std::endl;
  std::cerr << "  gear_ratio       减速比 (默认: 56)" << std::endl;
  std::cerr << std::endl;
  std::cerr << "示例:" << std::endl;
  std::cerr << "  " << prog_name << " 71" << std::endl;
  std::cerr << "      使用默认参数，GPIO 71 -> gpiochip2 引脚 7" << std::endl;
  std::cerr << std::endl;
  std::cerr << "  " << prog_name << " 71 50 11 30" << std::endl;
  std::cerr << "      采样周期 50ms, PPR 11, 减速比 30" << std::endl;
}

/* ================== main ================== */

int main(int argc, char *argv[]) {
  if (argc < 2 || argc > 5) {
    print_usage(argv[0]);
    return 1;
  }

  int gpio_number = std::atoi(argv[1]);
  double sample_period_ms = (argc >= 3) ? std::atof(argv[2]) : 33.0;
  double encoder_ppr = (argc >= 4) ? std::atof(argv[3]) : 11.0;
  double gear_ratio = (argc >= 5) ? std::atof(argv[4]) : 56.0;

  if (gpio_number < 0) {
    std::cerr << "错误: GPIO 编号必须为非负整数" << std::endl;
    return 1;
  }

  if (sample_period_ms <= 0) {
    std::cerr << "错误: 采样周期必须大于 0" << std::endl;
    return 1;
  }

  EncoderSpeedMeter meter(gpio_number,
                          sample_period_ms / 1000.0, // 转换为秒
                          encoder_ppr, gear_ratio);

  meter.print_info();

  g_meter = &meter;
  std::signal(SIGINT, sigint_handler);

  std::cout << "开始测量... (按 Ctrl+C 停止)" << std::endl;
  meter.start();

  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  return 0;
}

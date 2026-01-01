#include <gpiod.hpp>

#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>


/* ================== EncoderSpeedMeter ================== */

class EncoderSpeedMeter {
public:
    EncoderSpeedMeter(
        unsigned int gpio_offset,
        const std::string& chip_path = "/dev/gpiochip0",
        double sample_period = 0.033,
        double encoder_ppr = 11.0,
        double gear_ratio = 56.0,
        size_t queue_size = 10)
        : gpio_offset_(gpio_offset),
          chip_path_(chip_path),
          sample_period_(sample_period),
          encoder_ppr_(encoder_ppr),
          gear_ratio_(gear_ratio),
          queue_size_(queue_size) {}

    void start()
    {
        stop_flag_ = false;
        interrupt_thread_ = std::thread(&EncoderSpeedMeter::interrupt_loop, this);
        sampler_thread_   = std::thread(&EncoderSpeedMeter::sampler_loop, this);
        processor_thread_ = std::thread(&EncoderSpeedMeter::processor_loop, this);
    }

    void stop()
    {
        stop_flag_ = true;
        if (interrupt_thread_.joinable()) interrupt_thread_.join();
        if (sampler_thread_.joinable())   sampler_thread_.join();
        if (processor_thread_.joinable()) processor_thread_.join();
    }

private:
    /* ---------- GPIO ---------- */
    unsigned int gpio_offset_;
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
    /* ================== GPIO 中断监听线程 ================== */
    void interrupt_loop()
    {
        gpiod::chip chip(chip_path_);
        gpiod::line line = chip.get_line(gpio_offset_);

        line.request(
            {
                "encoder",
                gpiod::line_request::EVENT_BOTH_EDGES,
                gpiod::line_request::FLAG_BIAS_PULL_UP
            });

        while (!stop_flag_) {
            if (!line.event_wait(std::chrono::milliseconds(100)))
                continue;

            gpiod::line_event event = line.event_read();
            handle_event(event);
        }
    }

    /* ================== 中断逻辑 ================== */
    void handle_event(const gpiod::line_event& event)
    {
        static auto last_time = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        auto dt  = std::chrono::duration<double, std::milli>(now - last_time).count();
    
        // 如果距离上次事件小于最小间隔（例如 2ms），则忽略
        if (dt < 2.0)  // 2毫秒，可根据编码器特性调整
            return;
    
        last_time = now;
    
        std::lock_guard<std::mutex> guard(lock_);
        pulse_count_++;
    }
    

    /* ================== 采样线程 ================== */
    void sampler_loop()
    {
        auto last = std::chrono::steady_clock::now();

        while (!stop_flag_) {
            std::this_thread::sleep_for(
                std::chrono::duration<double>(sample_period_));

            auto now = std::chrono::steady_clock::now();
            std::chrono::duration<double> dt = now - last;
            last = now;

            uint64_t pulse = 0;
            {
                std::lock_guard<std::mutex> guard(lock_);

                pulse = pulse_count_;
                pulse_count_ = 0;
            }

            if (queue_.size() < queue_size_) {
                queue_.emplace(dt.count(), pulse);
            }
        }
    }

    /* ================== 计算线程 ================== */
    void processor_loop()
    {
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
            double motor_turns   = encoder_turns / gear_ratio_;
            double rps           = motor_turns / dt;

            printf(
                "脉冲数: %lu | dt: %.3f ms | 转速: %.2f 转/s | 转速: %.2f RPM\n",
                pulse,
                dt * 1000.0,
                rps,
                rps * 60.0
            );
        }
    }
};

/* ================== main ================== */

static EncoderSpeedMeter* g_meter = nullptr;

void sigint_handler(int)
{
    if (g_meter)
        g_meter->stop();
    std::exit(0);
}

int main()
{
    EncoderSpeedMeter meter(
        71,
        "/dev/gpiochip0",
        0.033,
        11.0,
        30.0);

    g_meter = &meter;
    std::signal(SIGINT, sigint_handler);

    std::cout << "开始测量..." << std::endl;
    meter.start();

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

#include <gpiod.h>
#include <chrono>
#include <thread>
#include <cmath>
#include <iostream>

constexpr const char* CHIP_NAME = "/dev/gpiochip0";
constexpr int GPIO_PIN = 71;
constexpr int PWM_FREQ_HZ = 500;      // PWM 频率
constexpr int BREATH_PERIOD_MS = 2000; // 呼吸周期，单位毫秒

int main() {
    // 打开 GPIO 芯片
    gpiod_chip* chip = gpiod_chip_open(CHIP_NAME);
    if (!chip) {
        std::cerr << "Failed to open GPIO chip\n";
        return 1;
    }

    // 获取对应线
    gpiod_line* line = gpiod_chip_get_line(chip, GPIO_PIN);
    if (!line) {
        std::cerr << "Failed to get GPIO line\n";
        gpiod_chip_close(chip);
        return 1;
    }

    // 请求输出模式
    if (gpiod_line_request_output(line, "breath_led", 0) < 0) {
        std::cerr << "Failed to request line as output\n";
        gpiod_chip_close(chip);
        return 1;
    }

    std::cout << "Starting PWM LED breath...\n";

    // PWM 循环
    while (true) {
        auto start = std::chrono::steady_clock::now();
        while (true) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
            
            // 呼吸周期计算 0~1 的占空比
            double phase = (elapsed_ms % BREATH_PERIOD_MS) / double(BREATH_PERIOD_MS);
            double brightness = 0.5 * (1 - cos(2 * M_PI * phase)); // cos 曲线更平滑
            
            // 计算高电平时间
            int high_time_us = int(brightness * 1e6 / PWM_FREQ_HZ);
            int low_time_us = int(1e6 / PWM_FREQ_HZ - high_time_us);

            // PWM 输出
            gpiod_line_set_value(line, 1);
            std::this_thread::sleep_for(std::chrono::microseconds(high_time_us));
            gpiod_line_set_value(line, 0);
            std::this_thread::sleep_for(std::chrono::microseconds(low_time_us));
        }
    }

    gpiod_line_release(line);
    gpiod_chip_close(chip);
    return 0;
}

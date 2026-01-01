#include <gpiod.h>
#include <iostream>
#include <chrono>
#include <thread>

constexpr const char* CHIP_NAME = "/dev/gpiochip0";
constexpr int GPIO_PIN = 77;

int main() {
    // 打开 GPIO 芯片
    gpiod_chip* chip = gpiod_chip_open(CHIP_NAME);
    if (!chip) {
        std::cerr << "Failed to open GPIO chip\n";
        return 1;
    }

    // 获取对应 GPIO 线
    gpiod_line* line = gpiod_chip_get_line(chip, GPIO_PIN);
    if (!line) {
        std::cerr << "Failed to get GPIO line\n";
        gpiod_chip_close(chip);
        return 1;
    }

    // 请求输入模式，监听下降沿（高->低）事件
    if (gpiod_line_request_falling_edge_events(line, "gpio_input_monitor") < 0) {
        std::cerr << "Failed to request falling edge events\n";
        gpiod_chip_close(chip);
        return 1;
    }

    std::cout << "Monitoring GPIO " << GPIO_PIN << " for low level...\n";

    gpiod_line_event event;
    while (true) {
        // 等待事件，超时 1 秒，避免永久阻塞
        int ret = gpiod_line_event_wait(line, nullptr);
        if (ret < 0) {
            std::cerr << "Error waiting for event\n";
            break;
        } else if (ret == 0) {
            // 超时，没有事件
            continue;
        }

        // 读取事件
        if (gpiod_line_event_read(line, &event) < 0) {
            std::cerr << "Failed to read event\n";
            break;
        }

        if (event.event_type == GPIOD_LINE_EVENT_FALLING_EDGE) {
            std::cout << "GPIO " << GPIO_PIN << " went LOW!" << std::endl;
        }
    }

    gpiod_line_release(line);
    gpiod_chip_close(chip);
    return 0;
}

#include <gpiod.h>
#include <iostream>
#include <thread>

constexpr const char* CHIP_NAME = "/dev/gpiochip0";
constexpr int GPIO_PIN = 71;

void handle_interrupt(gpiod_line* line) {
    gpiod_line_event event;
    while (true) {
        // 阻塞等待事件
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

        if (event.event_type == GPIOD_LINE_EVENT_RISING_EDGE) {
            std::cout << "GPIO " << GPIO_PIN << " went HIGH!" << std::endl;
        } else if (event.event_type == GPIOD_LINE_EVENT_FALLING_EDGE) {
            std::cout << "GPIO " << GPIO_PIN << " went LOW!" << std::endl;
        }
    }
}

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

    // 请求中断事件：上升沿和下降沿
    if (gpiod_line_request_both_edges_events(line, "gpio_interrupt") < 0) {
        std::cerr << "Failed to request both edge events\n";
        gpiod_chip_close(chip);
        return 1;
    }

    std::cout << "Waiting for GPIO " << GPIO_PIN << " events...\n";

    // 启动中断处理
    handle_interrupt(line);

    gpiod_line_release(line);
    gpiod_chip_close(chip);
    return 0;
}

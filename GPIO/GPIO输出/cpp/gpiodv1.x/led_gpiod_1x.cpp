#include <gpiod.h>
#include <iostream>
#include <unistd.h>

int main()
{
    const char *chipname = "/dev/gpiochip0";
    const unsigned int line_num = 71;

    // 打开 gpiochip
    gpiod_chip *chip = gpiod_chip_open(chipname);
    if (!chip) {
        perror("gpiod_chip_open");
        return 1;
    }

    // 获取 GPIO line
    gpiod_line *line = gpiod_chip_get_line(chip, line_num);
    if (!line) {
        perror("gpiod_chip_get_line");
        gpiod_chip_close(chip);
        return 1;
    }

    // 请求该 GPIO 为输出
    if (gpiod_line_request_output(line, "led_example", 0) < 0) {
        perror("gpiod_line_request_output");
        gpiod_chip_close(chip);
        return 1;
    }

    std::cout << "Blinking LED on GPIO " << line_num << std::endl;

    // 闪烁 10 次
    for (int i = 0; i < 10; ++i) {
        gpiod_line_set_value(line, 1); // LED ON
        usleep(500000);                // 500 ms
        gpiod_line_set_value(line, 0); // LED OFF
        usleep(500000);
    }

    // 释放资源
    gpiod_line_release(line);
    gpiod_chip_close(chip);

    return 0;
}

#include <gpiod.hpp>
#include <chrono>
#include <thread>
#include <iostream>

int main()
{
    gpiod::chip chip("/dev/gpiochip0");
    auto line = chip.get_line(71);

    line.request({
        "led_example",
        gpiod::line_request::DIRECTION_OUTPUT,
        0
    });

    for (int i = 0; i < 10; ++i) {
        line.set_value(1);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        line.set_value(0);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return 0;
}

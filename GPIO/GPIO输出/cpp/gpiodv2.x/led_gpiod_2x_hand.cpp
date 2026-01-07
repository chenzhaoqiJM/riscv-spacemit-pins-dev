#include <gpiod.hpp>
#include <chrono>
#include <thread>
#include <iostream>
#include <string>
#include <cstdlib>

void print_usage(const char* prog_name)
{
    std::cerr << "用法: " << prog_name << " <chip_index> <line_offset>" << std::endl;
    std::cerr << "示例: " << prog_name << " 2 19" << std::endl;
    std::cerr << "      表示使用 /dev/gpiochip2 的第 19 号引脚" << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc != 3) {
        print_usage(argv[0]);
        return 1;
    }

    int chip_index = std::atoi(argv[1]);
    int line_offset = std::atoi(argv[2]);

    std::string chip_path = "/dev/gpiochip" + std::to_string(chip_index);

    std::cout << "使用 " << chip_path << " 引脚 " << line_offset << std::endl;

    // libgpiod 2.x C++ API
    // 打开 GPIO 芯片
    auto chip = gpiod::chip(chip_path);

    // 配置引脚为输出，初始值低电平
    auto settings = gpiod::line_settings();
    settings.set_direction(gpiod::line::direction::OUTPUT);
    settings.set_output_value(gpiod::line::value::INACTIVE);

    auto req_builder = chip.prepare_request();
    req_builder.set_consumer("led_example");
    req_builder.add_line_settings(line_offset, settings);

    auto request = req_builder.do_request();

    std::cout << "LED 闪烁 10 次..." << std::endl;

    for (int i = 0; i < 10; ++i) {
        request.set_value(line_offset, gpiod::line::value::ACTIVE);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        request.set_value(line_offset, gpiod::line::value::INACTIVE);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::cout << "完成" << std::endl;
    return 0;
}

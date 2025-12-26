#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

const std::string SOCKET_PATH = "/run/pwm_cpp.sock";

int main() {
    // 创建 UNIX domain socket
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH.c_str(), sizeof(addr.sun_path) - 1);

    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        return 1;
    }

    std::cout << "Enter duty cycle (0.0 ~ 1.0), or 'q' to quit:\n";

    while (true) {
        std::string input;
        std::cout << "> ";
        std::getline(std::cin, input);

        if (input == "q" || input == "Q") break;

        try {
            double duty = std::stod(input);
            if (duty < 0.0 || duty > 1.0) {
                std::cout << "Invalid value. Must be between 0.0 and 1.0\n";
                continue;
            }

            // 构造 JSON
            json j;
            j["duty"] = duty;
            std::string msg = j.dump();

            // 发送 JSON
            if (write(sockfd, msg.c_str(), msg.size()) < 0) {
                perror("write");
                break;
            }

            // 接收回复
            char buffer[256];
            int n = read(sockfd, buffer, sizeof(buffer)-1);
            if (n > 0) {
                buffer[n] = '\0';
                std::cout << "Server reply: " << buffer << std::endl;
            }

        } catch (std::exception &e) {
            std::cout << "Invalid input: " << e.what() << std::endl;
        }
    }

    close(sockfd);
    return 0;
}
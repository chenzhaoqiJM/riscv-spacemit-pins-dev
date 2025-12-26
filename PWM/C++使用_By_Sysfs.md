# C++示例

## 安装依赖

```
sudo apt install nlohmann-json3-dev
```



## 服务端

```c++
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <algorithm>

#include "nlohmann/json.hpp"  // JSON 库

using json = nlohmann::json;

const std::string PWMCHIP = "/sys/class/pwm/pwmchip0";
const int PWM_INDEX = 0;
const std::string PWM_BASE = PWMCHIP + "/pwm" + std::to_string(PWM_INDEX);
const std::string SOCKET_PATH = "/run/pwm_cpp.sock";

// ================== 文件操作 ==================
void write_file(const std::string &path, const std::string &value) {
    std::ofstream ofs(path);
    if (!ofs) throw std::runtime_error("Failed to open " + path);
    ofs << value;
}

bool exists(const std::string &path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

void ensure_pwm_exported() {
    if (!exists(PWM_BASE)) {
        write_file(PWMCHIP + "/export", std::to_string(PWM_INDEX));
        for (int i = 0; i < 20; ++i) {
            if (exists(PWM_BASE)) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        if (!exists(PWM_BASE))
            throw std::runtime_error(PWM_BASE + " not created");
    }
}

// ================== IPC 服务 ==================
int setup_unix_socket(const std::string &path) {
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) throw std::runtime_error("socket() failed");

    // 删除已存在的 socket 文件
    unlink(path.c_str());

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        throw std::runtime_error("bind() failed");

    if (chmod(path.c_str(), 0666) < 0)
        throw std::runtime_error("chmod() failed");

    if (listen(server_fd, 5) < 0)
        throw std::runtime_error("listen() failed");

    return server_fd;
}

double parse_duty(const std::string &msg) {
    try {
        json j = json::parse(msg);
        double duty = j.value("duty", 0.0);
        return std::clamp(duty, 0.0, 1.0);
    } catch (...) {
        return 0.0;
    }
}

int main() {
    try {
        // ================== PWM 初始化 ==================
        ensure_pwm_exported();
        int period = 1'000'000; // ns
        write_file(PWM_BASE + "/enable", "0");
        write_file(PWM_BASE + "/period", std::to_string(period));
        write_file(PWM_BASE + "/duty_cycle", "0");
        write_file(PWM_BASE + "/enable", "1");

        // ================== UNIX SOCKET ==================
        int server_fd = setup_unix_socket(SOCKET_PATH);

        while (true) {
            int client_fd = accept(server_fd, nullptr, nullptr);
            if (client_fd < 0) continue;

            std::cout << "Client connected\n";

            while (true) {
                char buffer[256];
                int n = read(client_fd, buffer, sizeof(buffer)-1);

                if (n <= 0) {
                    // 客户端关闭连接
                    std::cout << "Client disconnected\n";
                    break;
                }

                buffer[n] = '\0';
                double duty = parse_duty(buffer);
                write_file(PWM_BASE + "/duty_cycle", std::to_string(static_cast<int>(1'000'000 * duty)));

                json reply = {{"ok", true}};
                std::string reply_str = reply.dump();
                write(client_fd, reply_str.c_str(), reply_str.size());
            }

            close(client_fd);
        }


    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
```

编译

```
gcc pwm_server.cpp -o pwm_server -lstdc++ -std=c++17
```

运行

```
./pwm_server
```



## 客户端

```c++
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
```

编译

```
g++ pwm_client.cpp -o pwm_client -lstdc++ -std=c++17
```

运行

```
./pwm_client
```

输出

```
➜  ~ ./pwmcontrl
Enter duty cycle (0.0 ~ 1.0), or 'q' to quit:
>
```

可以循环输入，控制占空比
# C++示例

## 安装依赖

```
sudo apt install nlohmann-json3-dev
```



## 服务端


编译

```
gcc pwm_server.cpp -o pwm_server -lstdc++ -std=c++17
```

运行

```
./pwm_server
```



## 客户端


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
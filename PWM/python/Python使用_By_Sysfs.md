# 从Python使用

## 安装依赖

```
sudo apt install python3-zmq
```



## 服务脚本

在 root 用户下运行，或则注册为启动时服务

### systemd 服务化

```
[Service]
ExecStart=/usr/bin/python3 /usr/local/bin/pwm_server.py
Restart=always
```



## 客户端

可以在普通用户下运行



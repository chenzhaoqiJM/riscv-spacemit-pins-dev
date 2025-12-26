# 从Python使用

## 安装依赖

```
sudo apt install python3-zmq
```



## 服务脚本

在 root 用户下运行，或则注册为启动时服务

```
# 以 root 启动
import zmq
import os
import time

PWMCHIP = "/sys/class/pwm/pwmchip0"
PWM_INDEX = 0
PWM_BASE = f"{PWMCHIP}/pwm{PWM_INDEX}"

def write(path, value):
    with open(path, "w") as f:
        f.write(str(value))

def ensure_pwm_exported():
    if not os.path.exists(PWM_BASE):
        # export
        write(f"{PWMCHIP}/export", PWM_INDEX)

        # 等待内核创建 pwmX 目录（很重要）
        for _ in range(20):
            if os.path.exists(PWM_BASE):
                break
            time.sleep(0.05)
        else:
            raise RuntimeError(f"{PWM_BASE} not created")

# ================== PWM 初始化 ==================
ensure_pwm_exported()

period = 1_000_000  # ns
write(f"{PWM_BASE}/enable", 0)          # 先关
write(f"{PWM_BASE}/period", period)
write(f"{PWM_BASE}/duty_cycle", 0)
write(f"{PWM_BASE}/enable", 1)

# ================== ZMQ 服务 ==================
ctx = zmq.Context()
sock = ctx.socket(zmq.REP)
sock.bind("ipc:///run/pwm.sock")
os.chmod("/run/pwm.sock", 0o666)

while True:
    msg = sock.recv_json()
    duty = msg.get("duty", 0.0)

    # 参数校验（非常重要）
    duty = max(0.0, min(1.0, float(duty)))

    write(f"{PWM_BASE}/duty_cycle", int(period * duty))

    sock.send_json({"ok": True})

```

### systemd 服务化

```
[Service]
ExecStart=/usr/bin/python3 /usr/local/bin/pwm_server.py
Restart=always
```



## 客户端

可以在普通用户下运行

```
import zmq

ctx = zmq.Context()
sock = ctx.socket(zmq.REQ)
sock.connect("ipc:///run/pwm.sock")

print("输入 duty (0.0 ~ 1.0)，输入 q 退出")

while True:
    s = input("duty> ").strip()

    if s.lower() in ("q", "quit", "exit"):
        break

    try:
        duty = float(s)
    except ValueError:
        print("错误，请输入数字")
        continue

    if not 0.0 <= duty <= 1.0:
        print("错误，duty 必须在 0.0 ~ 1.0 之间")
        continue

    # REQ → REP：send 后必须 recv
    sock.send_json({"duty": duty})
    reply = sock.recv_json()

    print("✔ 回复:", reply)

```


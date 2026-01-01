# CAN 调试方法

本文档介绍如何在 Linux 环境下使用 `socketcan` 工具（`ip`命令, `cansend`, `candump`）调试 CAN 接口。

## 1. 准备工作

确保您的系统已经安装了 `can-utils` 工具包。如果未安装，可以使用以下命令安装（以 Debian/Ubuntu 为例）：

```bash
sudo apt-get update
sudo apt-get install can-utils
```

## 2. 配置 CAN 接口

在使用 CAN 接口之前，需要先设置波特率并启用接口。假设我们要使用的接口是 `can0`，波特率设置为 1Mbps (1000000)。

### 关闭接口
在进行配置更改之前，建议先关闭接口：

```bash
sudo ip link set can0 down
```

### 设置波特率
设置 `can0` 的类型为 `can`，波特率为 1000000 bps：

```bash
sudo ip link set can0 type can bitrate 1000000
```

*注意：如果需要开启仲裁段波特率和数据段波特率（CAN FD），需要额外的参数，例如 `dbitrate` 和 `fd on`。*

### 启用接口
配置完成后，启动接口：

```bash
sudo ip link set can0 up
```

### 查看接口状态
使用以下命令查看 CAN 接口的详细信息：

```bash
ip -details link show can0
```

## 3. 发送数据 (cansend)

使用 `cansend` 命令发送 CAN 帧。格式为 `<interface> <can_id>#<data>`。

### 发送标准帧
发送 ID 为 `0x123`，数据为 `0x11 0x22 0x33 0x44 0x55 0x66 0x77 0x88` 的标准帧：

```bash
cansend can0 123#1122334455667788
```

### 发送扩展帧
发送扩展 ID `0x12345678`：

```bash
cansend can0 12345678#DEADBEEF
```

## 4. 接收数据 (candump)

使用 `candump` 命令监听并打印 CAN 总线上的数据。

### 监听所有数据
```bash
candump can0
```

### 显示详细信息
显示时间戳、ID 和数据：

```bash
candump -t a can0
```

## 5. 常见问题排查

- **接口无法启动 (`RTNETLINK answers: Device or resource busy`)**：接口可能已经处于 UP 状态，请先执行 `ip link set can0 down`。
- **无法接收数据**：
    - 检查波特率是否匹配。
    - 检查物理连接（H/L 线是否接反，终端电阻是否匹配）。
    - 检查是否有其他设备在总线上发送数据。

# GPIO 脉冲计数器 (libgpiod v2.x)

基于 **libgpiod v2.x C++ API** 的编码器脉冲计数和转速测量程序。

## 功能

- 通过 GPIO 边沿事件监听编码器脉冲
- 实时计算电机转速 (RPM)
- 支持命令行参数，自动计算 gpiochip 和引脚偏移
- 多线程架构：中断监听 + 采样 + 计算

## 编译

```bash
g++ -o pulse_gpiod_v2 pulse_gpiod_v2.cpp -lgpiodcxx -lgpiod -std=c++17
```

## 用法

```bash
./pulse_gpiod_v2 <gpio_number> [sample_period_ms] [encoder_ppr] [gear_ratio]
```

### 参数说明

| 参数 | 说明 | 默认值 |
|------|------|--------|
| `gpio_number` | GPIO 编号 (必须) | - |
| `sample_period_ms` | 采样周期 (毫秒) | 33 |
| `encoder_ppr` | 编码器每转脉冲数 | 11 |
| `gear_ratio` | 减速比 | 56 |

### 示例

```bash
# 使用默认参数，监听 GPIO 71
./pulse_gpiod_v2 71

# 自定义参数：GPIO 71, 采样周期 50ms, PPR 11, 减速比 30
./pulse_gpiod_v2 71 50 11 30
```

### GPIO 自动计算

程序会自动根据 GPIO 编号计算 gpiochip 和引脚偏移：

```
GPIO 71 -> /dev/gpiochip2 引脚 7   (71 / 32 = 2, 71 % 32 = 7)
GPIO 83 -> /dev/gpiochip2 引脚 19  (83 / 32 = 2, 83 % 32 = 19)
```

## 输出示例

```
GPIO 71 -> /dev/gpiochip2 引脚 7
采样周期: 33 ms
编码器 PPR: 11
减速比: 56
开始测量... (按 Ctrl+C 停止)
开始监听 GPIO 71 (/dev/gpiochip2 引脚 7)...
脉冲数: 12 | dt: 33.125 ms | 转速: 35.42 RPM
脉冲数: 11 | dt: 33.089 ms | 转速: 32.50 RPM
...
```

## 原理

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│ interrupt_loop  │───▶│  sampler_loop   │───▶│ processor_loop  │
│ (GPIO 边沿监听) │    │  (定时采样)      │    │  (转速计算)      │
└─────────────────┘    └─────────────────┘    └─────────────────┘
        │                      │                      │
        ▼                      ▼                      ▼
   脉冲计数++           读取并清零脉冲           RPM = (脉冲/PPR/减速比)/dt
```

## 依赖

- libgpiod v2.x
- libgpiodcxx (C++ bindings)

## 许可

MIT License

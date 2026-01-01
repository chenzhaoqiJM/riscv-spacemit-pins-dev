import gpiod
from gpiod.line import Direction, Edge

# 定义配置参数
CHIP_PATH = "/dev/gpiochip0"
LINE_OFFSET = 77

def monitor_low_level():
    # 1. 打开 GPIO 控制器
    with gpiod.Chip(CHIP_PATH) as chip:
        
        # 2. 准备线路配置：设置为输入，并监测下降沿（高到低）
        line_settings = gpiod.LineSettings(
            direction=Direction.INPUT,
            edge_detection=Edge.FALLING
        )
        
        # 3. 请求线路资源
        line_request = chip.request_lines(
            config={
                LINE_OFFSET: line_settings
            }
        )

        print(f"开始监听 {CHIP_PATH} 引脚 {LINE_OFFSET} 的低电平信号...")

        try:
            with line_request:
                while True:
                    # 4. 等待边沿事件（阻塞模式）
                    # wait_edge_events 返回 True 表示有事件发生
                    if line_request.wait_edge_events():
                        for event in line_request.read_edge_events():
                            # 当捕获到下降沿事件时
                            print(f"检测到输入低电平！(时间戳: {event.timestamp_ns} ns)")
        except KeyboardInterrupt:
            print("\n程序已停止")

if __name__ == "__main__":
    monitor_low_level()
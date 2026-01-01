import gpiod
from gpiod.line import Edge, Direction

# 定义配置
CHIP_PATH = "/dev/gpiochip0"
LINE_OFFSET = 71

def gpio_interrupt_demo():
    # 1. 打开 GPIO 控制器
    with gpiod.Chip(CHIP_PATH) as chip:
        
        # 2. 配置引脚：输入模式，检测上升沿 (Rising Edge)
        # 你也可以根据需要选择 Edge.FALLING 或 Edge.BOTH
        line_config = {
            LINE_OFFSET: gpiod.LineSettings(
                direction=Direction.INPUT,
                edge_detection=Edge.BOTH
            )
        }

        # 3. 请求线路
        with chip.request_lines(config=line_config) as request:
            print(f"正在监听 {CHIP_PATH} 引脚 {LINE_OFFSET} 的中断事件...")
            print("按 Ctrl+C 退出")

            try:
                while True:
                    # 4. 等待事件发生（设置超时时间，防止进程死锁）
                    if request.wait_edge_events(timeout=None):
                        # 读取触发的事件
                        events = request.read_edge_events()
                        for event in events:
                            print(event.event_type)
                            event_type = "上升沿" if event.event_type == event.Type.RISING_EDGE else "下降沿"
                            print(f"检测到中断！类型: {event_type}, 时间戳: {event.timestamp_ns} ns")
            except KeyboardInterrupt:
                print("\n程序已停止")

if __name__ == "__main__":
    gpio_interrupt_demo()
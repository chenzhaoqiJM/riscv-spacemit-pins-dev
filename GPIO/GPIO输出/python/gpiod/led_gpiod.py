import gpiod
import time
from gpiod.line import Direction, Value

# 定义配置参数
CHIP_PATH = "/dev/gpiochip0"
LINE_OFFSET = 71  # 对应硬件上的引脚编号

def led_blink():
    # 1. 打开 GPIO 控制器
    with gpiod.Chip(CHIP_PATH) as chip:
        
        # 2. 准备引脚配置：设置为输出模式
        line_config = {
            LINE_OFFSET: gpiod.LineSettings(
                direction=Direction.OUTPUT,
                output_value=Value.INACTIVE
            )
        }

        # 3. 请求该引脚（实例化请求对象）
        with chip.request_lines(config=line_config) as request:
            print(f"开始控制 {CHIP_PATH} 的引脚 {LINE_OFFSET}，按 Ctrl+C 退出...")
            
            try:
                while True:
                    # 设置为高电平 (ACTIVE)
                    request.set_value(LINE_OFFSET, Value.ACTIVE)
                    print("LED 亮")
                    time.sleep(1)
                    
                    # 设置为低电平 (INACTIVE)
                    request.set_value(LINE_OFFSET, Value.INACTIVE)
                    print("LED 灭")
                    time.sleep(1)
                    
            except KeyboardInterrupt:
                print("\n程序停止，正在释放资源...")

if __name__ == "__main__":
    led_blink()
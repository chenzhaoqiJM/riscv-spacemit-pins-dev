import gpiod
import time
from gpiod.line import Direction, Value

CHIP_PATH = "/dev/gpiochip0"
LINE_OFFSET = 71

def breathing_led():
    with gpiod.Chip(CHIP_PATH) as chip:
        line_config = {
            LINE_OFFSET: gpiod.LineSettings(
                direction=Direction.OUTPUT,
                output_value=Value.INACTIVE
            )
        }

        with chip.request_lines(config=line_config) as request:
            print(f"正在引脚 {LINE_OFFSET} 上模拟 PWM 呼吸灯...")
            
            # PWM 参数
            period = 0.01  # 周期时间：10ms (100Hz)
            steps = 50     # 呼吸的平滑度步长

            try:
                while True:
                    # 逐渐变亮
                    for i in range(steps + 1):
                        duty_cycle = i / steps  # 占空比 0.0 到 1.0
                        on_time = period * duty_cycle
                        off_time = period - on_time
                        
                        # 执行一个 PWM 周期
                        if on_time > 0:
                            request.set_value(LINE_OFFSET, Value.ACTIVE)
                            time.sleep(on_time)
                        if off_time > 0:
                            request.set_value(LINE_OFFSET, Value.INACTIVE)
                            time.sleep(off_time)

                    # 逐渐变暗
                    for i in range(steps, -1, -1):
                        duty_cycle = i / steps
                        on_time = period * duty_cycle
                        off_time = period - on_time
                        
                        if on_time > 0:
                            request.set_value(LINE_OFFSET, Value.ACTIVE)
                            time.sleep(on_time)
                        if off_time > 0:
                            request.set_value(LINE_OFFSET, Value.INACTIVE)
                            time.sleep(off_time)

            except KeyboardInterrupt:
                print("\n停止呼吸灯...")

if __name__ == "__main__":
    breathing_led()
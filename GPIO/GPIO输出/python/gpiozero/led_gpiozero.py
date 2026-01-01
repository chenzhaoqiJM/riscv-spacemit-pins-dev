from gpiozero.pins.lgpio import LGPIOFactory
from gpiozero import Device
Device.pin_factory = LGPIOFactory(chip=0)

from gpiozero import LED
import time

pin_number = 71

led1 = LED(pin_number)

try:
    while True:
        # 设置 GPIO 71 为高电平
        led1.on()
        print(f"GPIO {pin_number} ON")
        time.sleep(1)  # 等待 1 秒

        # 设置 GPIO 71 为低电平
        led1.off()
        print(f"GPIO {pin_number} OFF")
        time.sleep(1)  # 等待 1 秒

except KeyboardInterrupt:
    # 捕捉 Ctrl+C 并退出
    print("Exiting")

led1.close()
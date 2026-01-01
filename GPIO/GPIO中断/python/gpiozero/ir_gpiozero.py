
from gpiozero import Button
from signal import pause

# 1. 初始化引脚 (pin 71)
# pull_up=True: 内部上拉，按钮另一端接 GND（按下为低电平）
# bounce_time=0.05: 设置 50 毫秒的软件防抖
device = Button(71, pull_up=True, bounce_time=0.05)

# 2. 定义中断处理函数（回调函数）
def when_pressed():
    print("检测到中断：引脚变为低电平（按下）")

def when_released():
    print("检测到中断：引脚变为高电平（释放）")

# 3. 绑定事件
# 当引脚电平下降（Falling Edge）时触发
device.when_pressed = when_pressed
# 当引脚电平上升（Rising Edge）时触发
device.when_released = when_released

print("正在监听 71 号引脚中断信号...")

# 4. 保持程序运行，等待中断触发
pause()
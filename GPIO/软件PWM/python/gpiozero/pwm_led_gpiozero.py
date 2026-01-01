from gpiozero.pins.lgpio import LGPIOFactory
from gpiozero import Device
Device.pin_factory = LGPIOFactory(chip=0)
from gpiozero import PWMLED
from signal import pause

pin_number = 71

print(f"PWM {pin_number}")
led = PWMLED(pin_number)

led.pulse()

pause()
from gpiozero.pins.lgpio import LGPIOFactory
from gpiozero import Device
Device.pin_factory = LGPIOFactory(chip=0)

from gpiozero import Button
from signal import pause

pin_number = 77

print(f"Button {pin_number}")

def say_hello():
    print("Hello! Pressed")

def say_goodbye():
    print("Goodbye! Released")

button = Button(pin_number)

button.when_pressed = say_hello
button.when_released = say_goodbye

pause()
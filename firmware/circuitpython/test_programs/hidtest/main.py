import time
import board
import digitalio
from adafruit_hid.mouse import Mouse
from adafruit_hid.keyboard import Keyboard
from adafruit_hid.keycode import Keycode
from adafruit_hid.keyboard_layout_us import KeyboardLayoutUS

print("ARAM!")

btn = digitalio.DigitalInOut(board.SW1)
btn.direction = digitalio.Direction.INPUT
btn.pull = digitalio.Pull.UP

led_r = digitalio.DigitalInOut(board.LED2_R)
led_r.direction = digitalio.Direction.OUTPUT
led_g = digitalio.DigitalInOut(board.LED2_G)
led_g.direction = digitalio.Direction.OUTPUT
led_b = digitalio.DigitalInOut(board.LED2_B)
led_b.direction = digitalio.Direction.OUTPUT

led_r.value, led_g.value, led_b.value = True, True, True
# Set up a keyboard device.
kbd = Keyboard()
layout = KeyboardLayoutUS(kbd)

while True:
    if not btn.value:
        kbd.send(Keycode.GUI, Keycode.R)
        led_g.value = False
        time.sleep(0.5)
        layout.write('notepad\n')
        led_g.value = True
        led_b.value = False
        time.sleep(0.5)
        led_b.value = True
        layout.write('secret message')
        led_r.value = False
        time.sleep(3)
        led_r.value = True
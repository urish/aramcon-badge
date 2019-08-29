# Simple badge-based Clicker
# Copyright (C) 2019, Uri Shaked
#
# Usage:
# Left button - prev slide
# Middle button - play applause sound
# Right button - next slide

from adafruit_hid.keyboard import Keyboard
from adafruit_hid.keycode import Keycode
from badgeio import badge
import time

kbd = Keyboard()

while True:
    if badge.left:
        print("Left")
        badge.pixels.fill((0x10, 0, 0))
        kbd.press(Keycode.LEFT_ARROW)
        while badge.left:
            time.sleep(0.1)
        kbd.release_all()
        badge.pixels.fill(0x0)
        
    if badge.middle:
        badge.play_file('sounds/claps.mp3')
    
    if badge.right:
        print("Right")
        badge.pixels.fill((0, 0, 0x10))
        kbd.press(Keycode.RIGHT_ARROW)
        while badge.right:
            time.sleep(0.1)
        kbd.release_all()
        badge.pixels.fill(0x0)

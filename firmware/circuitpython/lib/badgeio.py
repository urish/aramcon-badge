# Released under The MIT License (MIT)
#
# Copyright (c) 2019 Uri Shaked

import board
import busio
from digitalio import DigitalInOut, Pull
from analogio import AnalogIn
import neopixel
import adafruit_lis3dh

class Badge:
    def __init__(self):
        self._left = DigitalInOut(board.LEFT_BUTTON)
        self._left.switch_to_input(pull=Pull.UP)
        self._middle = DigitalInOut(board.MIDDLE_BUTTON)
        self._middle.switch_to_input(pull=Pull.UP)
        self._right = DigitalInOut(board.RIGHT_BUTTON)
        self._right.switch_to_input(pull=Pull.UP)
        self._battery = AnalogIn(board.BATTERY_SENSE)
        self._led = DigitalInOut(board.LED)
        self._led.switch_to_output(value=True)
        self._vibration = DigitalInOut(board.VIBRATION_MOTOR)
        self._vibration.switch_to_output()
        self._pixels = neopixel.NeoPixel(board.NEOPIXELS, 4)
        self._i2c = busio.I2C(board.SCL, board.SDA)
        self._lis3dh = adafruit_lis3dh.LIS3DH_I2C(self._i2c, address=0x18)

    @property
    def left(self):
        """``True`` when the left button is pressed. ``False`` if not."""
        return not self._left.value

    @property
    def middle(self):
        """``True`` when the middle button is pressed. ``False`` if not."""
        return not self._middle.value

    @property
    def right(self):
        """``True`` when the right button is pressed. ``False`` if not."""
        return not self._right.value

    @property
    def red_led(self):
        """The red LED at the back of the board"""
        return not self._led.value

    @red_led.setter
    def red_led(self, value):
        self._led.value = not value

    @property
    def vibration(self):
        """Set to ``True`` to start vibrating"""
        return self._vibration.value

    @vibration.setter
    def vibration(self, value):
        self._vibration.value = value

    @property
    def pixels(self):
        """Array with the values of the 4 neopixels at the top of the board.

        See `neopixel.NeoPixel` for more info.

        .. code-block:: python
          from badgeio import badge
          import time

          badge.pixels.brightness = 0.5
          while True:
            badge.pixels[0] = (255, 0, 0) # Red
            time.sleep(0.5)
            badge.pixels[0] = (0, 255, 0) # Green
            time.sleep(0.5)
        """
        return self._pixels

    @property
    def acceleration(self):
        """Obtain acceleration as a tuple with 3 elements: (x, y, z)"""
        return self._lis3dh.acceleration

    @property
    def battery_voltage(self):
        """The battery voltage (if currently operating off the battery)"""
        return (self._battery.value * 3.3) / 65536

badge = Badge()

print("READY")
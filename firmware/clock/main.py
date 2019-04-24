import board
import digitalio
import busio
import time
import rtc
from adafruit_epd.epd import Adafruit_EPD
from adafruit_epd.il0373 import Adafruit_IL0373

import clock

# Initialize the SPI bus and setup the display
spi = busio.SPI(board.SCK, MISO=board.MISO, MOSI=board.MOSI)
display = Adafruit_IL0373(128, 296, spi,
    cs_pin=digitalio.DigitalInOut(board.DISP_CS),
    dc_pin=digitalio.DigitalInOut(board.DISP_DC),
    rst_pin=digitalio.DigitalInOut(board.DISP_RESET),
    busy_pin=digitalio.DigitalInOut(board.DISP_BUSY),
    sramcs_pin = None,
)
display.set_black_buffer(1, False)
display.set_color_buffer(1, False)

display.fill(Adafruit_EPD.BLACK)

rtc_instance = rtc.RTC()
clock.draw_time(display, rtc_instance.datetime)

display.display()
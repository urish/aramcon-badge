import board
import busio
import time
import rtc
import adafruit_il0373
import displayio
import clock
from adafruit_display_shapes.rect import Rect

displayio.release_displays()

spi = busio.SPI(board.SCK, MISO=board.MISO, MOSI=board.MOSI)
display_bus = displayio.FourWire(spi, command=board.DISP_DC, chip_select=board.DISP_CS, reset=board.DISP_RESET,
                                 baudrate=1000000)
time.sleep(1)
display = adafruit_il0373.IL0373(display_bus, width=296, height=128, 
                                 rotation=270, seconds_per_frame=5, busy_pin=board.DISP_BUSY)

while True:
  rtc_instance = rtc.RTC()
  group = displayio.Group()
  group.append(Rect(0, 0, display.width, display.height, fill=0xffffff))
  group.append(clock.draw_time(rtc_instance.datetime, 48, 40))
  display.show(group)
  display.wait_for_frame()
  time.sleep(60-time.time()%60)

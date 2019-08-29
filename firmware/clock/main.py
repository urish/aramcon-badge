import board
import busio
import time
import rtc
import adafruit_il0373
import displayio
import clock
from badgeio import badge
from adafruit_display_shapes.rect import Rect

displayio.release_displays()
display = badge.display

while True:
  rtc_instance = rtc.RTC()
  group = displayio.Group()
  group.append(Rect(0, 0, display.width, display.height, fill=0xffffff))
  group.append(clock.draw_time(rtc_instance.datetime, 48, 40))
  display.show(group)
  while display.time_to_refresh > 0:
    pass
  display.refresh()
  time.sleep(60-time.time()%60)

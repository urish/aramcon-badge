import time
import board
import displayio
import adafruit_il91874
from adafruit_focaltouch import Adafruit_FocalTouch
from badgeio import badge

touchscr = Adafruit_FocalTouch(badge.i2c)
red = 0
green = 0
blue = 0

displayio.release_displays()
display = adafruit_il91874.IL91874(badge.display_bus, width=264, height=176, rotation=270, 
  busy_pin=board.DISP_BUSY, highlight_color=0xff0000)

g = displayio.Group()

f = open("rgb.bmp", "rb")

pic = displayio.OnDiskBitmap(f)
t = displayio.TileGrid(pic, pixel_shader=displayio.ColorConverter())
g.append(t)

display.show(g)

display.refresh()

while True:
  if touchscr.touched:
    for touch in touchscr.touches:
      intensity = max((touch['y'] - 20) // 2, 0)
      if (touch['x'] < 58):
        red = intensity
      elif touch['x'] < 117:
        green = intensity
      else:
        blue = intensity
      print(touch['x'])
  badge.pixels.fill((red, green, blue))
  time.sleep(0.05)

import displayio
import terminalio
from adafruit_display_text import label
from badgeio import badge

badge_id = open('badge_id.txt').read().strip()

displayio.release_displays()
display = badge.display

frame = displayio.Group()

pic = displayio.OnDiskBitmap(open("assets/welcome.bmp", "rb"))
grid = displayio.TileGrid(pic, pixel_shader=displayio.ColorConverter())
frame.append(grid)

pic = displayio.OnDiskBitmap(open("assets/banner.bmp", "rb"))
grid = displayio.TileGrid(pic, pixel_shader=displayio.ColorConverter())
frame.append(grid)

text_area = label.Label(terminalio.FONT, text="Badge", color=0x0, x=84, y=20)
frame.append(text_area)

text_area = label.Label(terminalio.FONT, text="#{}".format(badge_id), color=0x0)
x_offset = 8 if len(badge_id) < 2 else 0
grp = displayio.Group(scale=2, x=84+x_offset, y=38)
grp.append(text_area)
frame.append(grp)

display.show(frame)
display.refresh()

while True:
    for i in range(4):
        badge.pixels[i] = (255 * badge.left, 255 * badge.middle, 255 * badge.right)
    badge.vibration = badge.left

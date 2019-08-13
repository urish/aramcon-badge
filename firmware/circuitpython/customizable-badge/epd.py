import displayio
from badgeio import badge
import terminalio
from adafruit_display_text import label

frame1 = None
frame2 = None

LINKEDIN_FRAME = 1
NAMETAG_FRAME = 2

def create_frames():
    global frame1
    global frame2

    frame1 = displayio.Group()
    pic = displayio.OnDiskBitmap(open("images/frame1.bmp", 'rb'))
    grid = displayio.TileGrid(pic, pixel_shader=displayio.ColorConverter())
    frame1.append(grid)
    text_area = label.Label(terminalio.FONT, text="NAME", color=0x0)
    text_area.x = 40
    text_area.y = 40
    frame1.append(text_area)

    frame2 = displayio.Group()
    pic = displayio.OnDiskBitmap(open("images/frame2.bmp", 'rb'))
    grid = displayio.TileGrid(pic, pixel_shader=displayio.ColorConverter())
    frame2.append(grid)

def display_frame(i):
    if i == LINKEDIN_FRAME:
        badge.display.show(frame1)
    elif i == NAMETAG_FRAME:
        badge.display.show(frame2)

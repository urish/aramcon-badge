# Nametags app for Aramcon Badge
# Copyright (C) 2019, Uri Shaked

from badgeio import badge
import adafruit_miniqr
import bleio
import displayio
import time

palette = displayio.Palette(2)
palette[0] = 0x000000
palette[1] = 0xFFFFFF

inverse_palette = displayio.Palette(2)
inverse_palette[0] = 0xFFFFFF
inverse_palette[1] = 0x000000

def banner():
    image = displayio.OnDiskBitmap(open("/assets/banner.bmp", "rb"))
    return displayio.TileGrid(image, pixel_shader=inverse_palette)

def bitmap_QR(matrix):
    BORDER_PIXELS  = 2
    bitmap = displayio.Bitmap(matrix.width+2*BORDER_PIXELS,
                              matrix.height+2*BORDER_PIXELS, 2)
    for y in range(matrix.height):
        for x in range(matrix.width):
            if matrix[x, y]:
                bitmap[x+BORDER_PIXELS, y+BORDER_PIXELS] = 1
            else:
                bitmap[x+BORDER_PIXELS, y+BORDER_PIXELS] = 0
    return bitmap

def get_qr_url():
    addr = bleio.adapter.address.address_bytes
    return 'https://go.urish.org/AC?b={:02X}{:02X}{:02X}'.format(addr[3], addr[4], addr[5])

def display_qr():
    qr = adafruit_miniqr.QRCode()
    qr.add_data(get_qr_url().encode('utf-8'))
    qr.make()
    qr_bitmap = bitmap_QR(qr.matrix)
    qr_img = displayio.TileGrid(qr_bitmap, pixel_shader=palette)
    qr_group = displayio.Group(scale=4, x=96, y=6)
    qr_group.append(qr_img)
    frame = displayio.Group()
    frame.append(qr_group)
    frame.append(banner())
    badge.display.show(frame)
    time.sleep(badge.display.time_to_refresh)
    badge.display.refresh()

def display_nametag():
    try:
        badge.show_bitmap('/nametag.bmp', inverse_palette)
        return True
    except:
        return False

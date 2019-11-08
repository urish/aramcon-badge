"""
Badge LED Matrix SAO Demo

The LED Matrix has 9x10 LEDs, and uses 18-byte buffer.
Every two bytes in the buffer control one row, and the
columns are controlled by individual bits (only 10 bits
out of 16 are actually used in each row). The byte
order is little-endian.
"""

from badgeio import badge
import time

badge.i2c.try_lock()

# Clear screen buffer
buf = bytearray([0] * 18)
badge.i2c.writeto(52, buf)

def back_and_forth(n):
    return list(range(n))+list(range(n - 2, 0, -1))

def scan_rows():
    for y in back_and_forth(9) + [0]:
        buf = bytearray([0] * 18)
        buf[y*2] = 0xff
        buf[y*2+1] = 0xff
        badge.i2c.writeto(52, buf)
        time.sleep(0.05)

def scan_cols():
    for x in back_and_forth(10) + [0]:
        buf = bytearray([0] * 18)
        for y in range(9):
            buf[y*2 + x // 8] = 1 << (x % 8)
        badge.i2c.writeto(52, buf)
        time.sleep(0.05)

def triangles():
    while not badge.left and not badge.right:
        buf = bytearray([0] * 18)
        for y in range(9):
            for x in range(10):
                if x <= y:
                    buf[y*2 + x // 8] |= 1 << (x % 8)
        badge.i2c.writeto(52, buf)
        time.sleep(0.3)

        buf = bytearray([0] * 18)
        for y in range(9):
            for x in range(10):
                if x >= y:
                    buf[y*2 + x // 8] |= 1 << (x % 8)
        badge.i2c.writeto(52, buf)
        time.sleep(0.3)


while True:
    if badge.left:
        scan_cols()
    if badge.middle:
        triangles()
    if badge.right:
        scan_rows()

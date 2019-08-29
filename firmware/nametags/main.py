# Nametags app for Aramcon Badge
# Copyright (C) 2019, Uri Shaked

import displayio
from badgeio import badge
import ui
import random
import time

def main():
    displayio.release_displays()
    ui.display_qr()
    while True:
        badge.vibration = badge.left
        if badge.middle:
            badge.pixels.fill(random.randrange(0xffffff))
            time.sleep(0.2)
        elif badge.right:
            badge.pixels.fill(0x0)

if __name__ == '__main__':
    main()

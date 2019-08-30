# Nametags app for Aramcon Badge
# Copyright (C) 2019, Uri Shaked

import random
import time
from badgeio import badge
from name_server import NameServer
import ui

def main():
    ui.display_qr()
    name = NameServer()
    name.start_advertising()
    while True:
        name.update()
        badge.vibration = badge.left
        if badge.middle:
            badge.pixels.fill(random.randrange(0xffffff))
            time.sleep(0.2)
        elif badge.right:
            badge.pixels.fill(0x0)

if __name__ == '__main__':
    main()

# Nametags app for Aramcon Badge
# Copyright (C) 2019, Uri Shaked

import random
import time
from badgeio import badge
from name_server import NameServer
import ui

def main():
    showing_name = ui.display_nametag()
    name = NameServer()
    if not showing_name:
        ui.display_qr()
        name.start_advertising()
    while True:
        name.update()
        if badge.left:
            while badge.left:
                pass
            if showing_name:
                showing_name = False
                ui.display_qr()
                name.start_advertising()
            else:
                showing_name = ui.display_nametag()
                if showing_name:
                    name.stop_advertising()
        if badge.middle:
            badge.pixels.fill(random.randrange(0xffffff))
            time.sleep(0.2)
        elif badge.right:
            badge.pixels.fill(0x0)

if __name__ == '__main__':
    main()

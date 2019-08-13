import epd
import neopixel_test
import sound_test
import time
from badgeio import badge

def main():
    sound_test.init(badge.spi)
    neopixel_test.status_pixel(True)
    sound_test.play()
    epd.create_frames()
    neopixel_test.status_pixel(False)

    print ("setup complete")

    while True:
        if badge.left:
            badge.vibration = True
            neopixel_test.status_pixel(True)
            sound_test.play()
            badge.vibration = False
            epd.display_frame(epd.NAMETAG_FRAME)
            neopixel_test.status_pixel(False)
        elif badge.middle:
            badge.vibration = True
            neopixel_test.status_pixel(True)
            sound_test.play()
            badge.vibration = False
            epd.display_frame(epd.LINKEDIN_FRAME)
            neopixel_test.status_pixel(False)
        elif badge.right:
            badge.vibration = True
            sound_test.play()
            badge.vibration = False
            neopixel_test.demo()
        else:
            pass
        time.sleep(0.1)

if __name__ == '__main__':
    main()

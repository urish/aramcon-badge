import epd
import neopixel_test
import sound_test
import board
import digitalio
import time
import busio

def main():
    vibration_motor = digitalio.DigitalInOut(board.VIBRATION_MOTOR)
    vibration_motor.direction = digitalio.Direction.OUTPUT

    button_left = digitalio.DigitalInOut(board.LEFT_BUTTON)
    button_left.direction = digitalio.Direction.INPUT
    button_left.pull = digitalio.Pull.UP

    button_middle = digitalio.DigitalInOut(board.MIDDLE_BUTTON)
    button_middle.direction = digitalio.Direction.INPUT
    button_middle.pull = digitalio.Pull.UP

    button_right = digitalio.DigitalInOut(board.RIGHT_BUTTON)
    button_right.direction = digitalio.Direction.INPUT
    button_right.pull = digitalio.Pull.UP
    spi = busio.SPI(board.SCK, MISO=board.MISO, MOSI=board.MOSI)
    sound_test.init(spi)

    neopixel_test.status_pixel(True)
    epd.init(spi)
    sound_test.play()
    epd.create_frames()
    neopixel_test.status_pixel(False)


    print ("setup complete")

    while True:
        if not button_left.value:
            vibration_motor.value = True
            neopixel_test.status_pixel(True)
            sound_test.play()
            vibration_motor.value = False
            epd.display_frame(epd.NAMETAG_FRAME)
            neopixel_test.status_pixel(False)
        elif not button_middle.value:
            vibration_motor.value = True
            neopixel_test.status_pixel(True)
            sound_test.play()
            vibration_motor.value = False
            epd.display_frame(epd.LINKEDIN_FRAME)
            neopixel_test.status_pixel(False)
        elif not button_right.value:
            vibration_motor.value = True
            sound_test.play()
            vibration_motor.value = False
            neopixel_test.demo()
        else:
            pass
        time.sleep(0.1)

if __name__ == '__main__':
    main()

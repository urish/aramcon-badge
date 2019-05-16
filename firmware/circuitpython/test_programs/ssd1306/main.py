import adafruit_ssd1306
import board
import busio
from digitalio import DigitalInOut
 
spi = busio.SPI(board.P1_10, MOSI=board.P1_13, MISO=None)
dc_pin = DigitalInOut(board.P0_02)    # any pin!
reset_pin = DigitalInOut(board.P1_15) # any pin!
cs_pin = DigitalInOut(board.P0_29)    # any pin!
 
oled = adafruit_ssd1306.SSD1306_SPI(128, 32, spi, dc_pin, reset_pin, cs_pin)
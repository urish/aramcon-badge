import board
import busio
from digitalio import DigitalInOut
from adafruit_rgb_display import color565
import adafruit_rgb_display.ili9341 as ili9341
 
spi = busio.SPI(clock=board.P1_10, MOSI=board.P1_13)
dc_pin = DigitalInOut(board.P0_02)    # any pin!
reset_pin = DigitalInOut(board.P1_15) # any pin!
cs_pin = DigitalInOut(board.P0_29)    # any pin!

display = ili9341.ILI9341(spi, cs=cs_pin, dc=dc_pin, rst=reset_pin)
display.fill(color565(255, 0, 0))
display.pixel(64, 64, 0)
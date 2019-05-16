import board
import digitalio
import busio
from vs1053 import Player
import adafruit_vs1053
import time

def getLed(pin):
    led = digitalio.DigitalInOut(pin)
    led.direction = digitalio.Direction.OUTPUT
    led.value = True
    return led
 
r = getLed(board.LED)
r.value = False

spi = busio.SPI(board.SCK, MISO=board.MISO, MOSI=board.MOSI)

player = Player(
    spi,
    xResetPin = board.SND_RESET,
    dReqPin = board.SND_DREQ,
    xDCSPin = board.SND_XDCS,
    xCSPin = board.SND_CS
)
player.setVolume(1)

print ("Play!")
inputFile = open('sound_samples/promo2.mp3', mode='rb')
buf = bytearray(32)
while inputFile.readinto(buf):
    player.writeData(buf)
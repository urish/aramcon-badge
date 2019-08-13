import board
import digitalio
from vs1053 import Player
import adafruit_vs1053
import time

player = None

def init(spi):
    global player
    player = Player(
        spi,
        xResetPin = board.SND_RESET,
        dReqPin = board.SND_DREQ,
        xDCSPin = board.SND_XDCS,
        xCSPin = board.SND_CS
    )
    player.setVolume(0.7)


def play():
    print ("Play!")
    inputFile = open('sound/sound1.mp3', mode='rb')
    buf = bytearray(32)
    while inputFile.readinto(buf):
        player.writeData(buf)
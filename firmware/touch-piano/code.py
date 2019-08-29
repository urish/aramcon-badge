# Keyboard Piano for Badge using MPR121 touch controller
# Copyright (C) 2019, Uri Shaked

from badgeio import badge
from adafruit_mpr121 import MPR121
import displayio
import time

displayio.release_displays()

mpr121 = MPR121(badge.i2c)
notes = [60, 62, 64, 65, 67, 69, 71, 72]
midi = badge.midi
while True:
    for i in range(len(notes)):
        if mpr121[i].value:
            midi.note_on(notes[i], 0x7f)
            time.sleep(0.1)

# Keyboard Piano for Badge using MPR121 touch controller
# Copyright (C) 2019, Uri Shaked

from badgeio import badge
from adafruit_mpr121 import MPR121
from adafruit_midi.program_change import ProgramChange
import time

badge.show_bitmap('music.bmp')

mpr121 = MPR121(badge.i2c)
notes = [60, 62, 64, 65, 67, 69, 71, 72]
midi = badge.midi
while True:
    badge.pixels.fill(0x0)
    if badge.left:
        midi.send(ProgramChange(10))
    if badge.middle:
        midi.send(ProgramChange(0))
    if badge.right:
        midi.send(ProgramChange(20))
    for i in range(len(notes)):
        if mpr121[i].value:
            badge.pixels[i % 4] = (0, 10, 10 * (i > 3))
            midi.note_on(notes[i], 0x7f)
            time.sleep(0.1)

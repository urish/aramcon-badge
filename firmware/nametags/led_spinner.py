# Nametags app for Aramcon Badge
# Copyright (C) 2019, Uri Shaked

import time

SPINNER_CYCLE = [
    [(0, 0, 50), (0, 0, 0,), (0, 0, 15), (0, 0, 0)],
    [(0, 0, 15), (0, 0, 50,), (0, 0, 0), (0, 0, 0)],
    [(0, 0, 0), (0, 0, 15,), (0, 0, 0), (0, 0, 50)],
    [(0, 0, 0), (0, 0, 0,), (0, 0, 50), (0, 0, 15)],
]

class LedSpinner:
    def __init__(self, pixels):
        self._idx = 0
        self._pixels = pixels
        self._last_time = time.monotonic()
        self.cycle = SPINNER_CYCLE
    
    def next(self):
        current_time = time.monotonic()
        if current_time - self._last_time < 0.2:
            return
        self._idx += 1
        self._pixels[:] = self.cycle[self._idx % len(self.cycle)]
        self._last_time = current_time

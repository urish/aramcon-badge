SPINNER_CYCLE = [
    [(0, 0, 50), (0, 0, 0,), (0, 0, 0), (0, 0, 0)],
    [(0, 0, 0), (0, 0, 50,), (0, 0, 0), (0, 0, 0)],
    [(0, 0, 0), (0, 0, 0,), (0, 0, 50), (0, 0, 0)],
    [(0, 0, 0), (0, 0, 0,), (0, 0, 0), (0, 0, 50)],
]

class LedSpinner:
    def __init__(self, pixels):
        self._idx = 0
        self._pixels = pixels
        self.cycle = SPINNER_CYCLE
    
    def next(self):
        self._idx += 1
        self._pixels[:] = self.cycle[self._idx % len(self.cycle)]

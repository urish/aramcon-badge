# Img2EPaper tool - converts a given image to an ePaper Display Bitmap
# Copyright (C) 2019, Uri Shaked. Provided under the terms of the MIT License.

import math
import sys
from PIL import Image

ELEMENTS_PER_LINE = 15

def img2epaper(img):
    for x in range(img.size[0] - 1, 0, -1):
        for y in range(math.floor(img.size[1] / 8)):
            val = 0
            for bit in range(8):
                if img.getpixel((x, y * 8 + bit)):
                    val = val | (1 << (7-bit))
            yield val

img = Image.open(sys.argv[1]).convert('1')
line = ""
for idx, val in enumerate(img2epaper(img)):
    line += ("0x%02x, " % val)
    if (idx + 1) % ELEMENTS_PER_LINE == 0:
        print(line)
        line = ""
if line:
    print (line)

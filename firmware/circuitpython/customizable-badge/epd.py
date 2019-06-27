import digitalio
import board
from adafruit_epd.epd import Adafruit_EPD
from adafruit_epd.il0373 import Adafruit_IL0373
import time

frame1 = None
frame2 = None

LINKEDIN_FRAME = 1
NAMETAG_FRAME = 2

# create the spi device and pins we will need
display = None

def init(spi):
    global display
    display = Adafruit_IL0373(128, 296, spi,
    cs_pin=digitalio.DigitalInOut(board.DISP_CS),
    dc_pin=digitalio.DigitalInOut(board.DISP_DC),
    rst_pin=digitalio.DigitalInOut(board.DISP_RESET),
    busy_pin=digitalio.DigitalInOut(board.DISP_BUSY),
    sramcs_pin = None,
    )

    display.set_black_buffer(1, False)
    display.set_color_buffer(1, False)

    display.rotation = 3

def read_le(s):
    # as of this writting, int.from_bytes does not have LE support, DIY!
    result = 0
    shift = 0
    for byte in bytearray(s):
        result += byte << shift
        shift += 8
    return result

class BMPError(Exception):
    pass

def display_bitmap(epd, filename, x, y): # pylint: disable=too-many-locals, too-many-branches
    try:
        f = open("/" + filename, "rb")
    except OSError:
        print("Couldn't open file")
        return

    print("File opened")
    try:
        if f.read(2) != b'BM':  # check signature
            raise BMPError("Not BitMap file")

        bmpFileSize = read_le(f.read(4))
        f.read(4)  # Read & ignore creator bytes

        bmpImageoffset = read_le(f.read(4))  # Start of image data
        headerSize = read_le(f.read(4))
        bmpWidth = read_le(f.read(4))
        bmpHeight = read_le(f.read(4))
        flip = True

        print("Size: %d\nImage offset: %d\nHeader size: %d" %
              (bmpFileSize, bmpImageoffset, headerSize))
        print("Width: %d\nHeight: %d" % (bmpWidth, bmpHeight))

        if read_le(f.read(2)) != 1:
            raise BMPError("Not singleplane")
        bmpDepth = read_le(f.read(2))  # bits per pixel
        print("Bit depth: %d" % (bmpDepth))
        if bmpDepth != 24:
            raise BMPError("Not 24-bit")
        if read_le(f.read(2)) != 0:
            raise BMPError("Compressed file")

        print("Image OK! Drawing...")

        rowSize = (bmpWidth * 3 + 3) & ~3  # 32-bit line boundary

        for row in range(bmpHeight):  # For each scanline...
            if flip:  # Bitmap is stored bottom-to-top order (normal BMP)
                pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize
            else:  # Bitmap is stored top-to-bottom
                pos = bmpImageoffset + row * rowSize

            # print ("seek to %d" % pos)
            f.seek(pos)
            rowdata = f.read(3*bmpWidth)
            for col in range(bmpWidth):
                b, g, r = rowdata[3*col:3*col+3]  # BMP files store RGB in BGR
                if r == 0 and g == 0 and b == 0:
                    epd.pixel(col + x, row + y, Adafruit_EPD.BLACK)
                else:
                    pass #epd.pixel(row, col, Adafruit_EPD.WHITE)
    except OSError:
        print("Couldn't read file")
    except BMPError as e:
        print("Failed to parse BMP: " + e.args[0])
    finally:
        f.close()
    print("Finished drawing")


def create_frames():
    global frame1
    global frame2

    display.fill(Adafruit_EPD.WHITE)
    display_bitmap(display, "images/frame1.bmp", 0 , 0)
    display.text("Benny Meisels", 40, 40, Adafruit_EPD.BLACK)
    frame1 = display._buffer2[:]
    print ("created frame1")

    display.fill(Adafruit_EPD.WHITE)
    display_bitmap(display, "images/frame2.bmp", 0 , 0)
    frame2 = display._buffer2[:]
    print ("created frame2")

def display_frame(i):
    if i == 1:
        display._buffer2[:] = frame1
        display.display()
    elif i == 2:
        display._buffer2[:] = frame2
        display.display()

    display.power_down()
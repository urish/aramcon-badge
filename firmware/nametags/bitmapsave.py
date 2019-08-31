# Nametags app for Aramcon Badge
# Copyright (C) 2019, Uri Shaked

def bitmap_save(file, bitmap):
    with open(file, 'wb') as out:
        bitmap_write(out, bitmap)

def bitmap_write(out, bitmap):
    line_size = bitmap.width // 8
    line_pad = 4 - (line_size % 4) if line_size % 4 > 0 else 0 
    line_size += line_pad
    # Bitmap Header
    out.write(b'BM' +
        (14+40+8+bitmap.height*line_size).to_bytes(4, 'little')
        + b'\0\0\0\0' + (14+40+8).to_bytes(4, 'little'))
    # BITMAPINFOHEADER
    out.write(b'(\0\0\0' + (bitmap.width).to_bytes(4, 'little') + 
                (bitmap.height).to_bytes(4, 'little') + b'\1\0\1\0\0\0\0\0' + 
                (bitmap.height*line_size).to_bytes(4, 'little') +
                b'\0\0\0\0\0\0\0\0\2\0\0\0\2\0\0\0')
    # 2-bit palette (black, white)
    out.write(b'\0\0\0\0\xff\xff\xff\0')
    # Actual bitmap data
    for y in range(bitmap.height):
        for x in range(bitmap.width // 8):
            value = 0x0
            offset = (bitmap.height - 1 - y) * bitmap.width + x * 8
            for bit in range(8):
                value |= bitmap[offset + bit] << (7 - bit)
            out.write(bytes([value]))
        out.write(b'\xff' * line_pad)

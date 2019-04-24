import displayio

def read_glyph(file):
  dims = file.readline()
  width, height = map(int, dims.strip().split(','))
  bitmap = displayio.Bitmap(width, height, 2)
  for y, line in enumerate(file):
    if y < height:
      for x in range(0, width):
        bitmap[x, y] = 1 if (line[x] == ' ') else 0
  return bitmap

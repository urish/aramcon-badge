import txtfont

ASSET_LIST = ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'colon']

# Load all assets
assets = {}
for asset in ASSET_LIST:
  with open('font/%s.txt' % asset, 'r') as file:
    assets[asset] = txtfont.read_glyph(file)
assets[':'] = assets['colon']

def draw_glyph(display, x, y, glyph):
  for dx in range(glyph.width):
    for dy in range(glyph.height):
      display.pixel(x + dx, y + dy, glyph[dx, dy])

def draw_time(display, dt):
  timestr = '%02d:%02d' % (dt.tm_hour, dt.tm_min)
  print("Update clock: %s" % timestr)
  y = display.height - 40
  x = 40
  spacing = 8
  for ch in timestr:
    y -= assets[ch].height + spacing
    draw_glyph(display, x, y, assets[ch])

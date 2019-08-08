import displayio

ASSET_LIST = ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'colon']

# Load all assets
assets = {}
for asset in ASSET_LIST:
  assets[asset] = displayio.OnDiskBitmap(open('font/%s.bmp' % asset, 'rb'))
assets[':'] = assets['colon']

def draw_time(dt, x, y):
  timestr = '%02d:%02d' % (dt.tm_hour, dt.tm_min)
  print("Update clock: %s" % timestr)
  group = displayio.Group(max_size=len(timestr), x=x, y=y)
  xpos = 0
  for ch in timestr:
    sprite = displayio.TileGrid(assets[ch], x=xpos, pixel_shader=displayio.ColorConverter())
    group.append(sprite)
    xpos += assets[ch].width + 8
  return group

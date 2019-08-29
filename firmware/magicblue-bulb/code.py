from magicblue import MagicblueBulb
from badgeio import badge

badge.pixels.fill((0, 0, 10))
bulb = MagicblueBulb()
bulb.connect()

try:
    badge.pixels.fill(0)
    while True:
        bulb.color = (badge.left * 255, badge.middle * 255, badge.right * 255)
except:
    badge.pixels.fill((10, 0, 0))

import re
spacing = 10

with open('cross-1.kicad_mod', 'r') as inp, open('../badge.pretty/cross-array.kicad_mod', 'w') as outp:
  for line in inp:
    if line.startswith('(fp_line') or line.startswith('(fp_text user'):
      for unit in range(16):
        scale = 1.0 + unit * 0.01
        x = spacing * (unit % 4)
        y = spacing * (unit // 4)
        scaled = re.sub(r'(-?\d.\d{3,}) (-?\d.\d{3,})', 
          lambda m: '{:.6f} {:.6f}'.format(float(m[1]) * scale + x, float(m[2]) * scale + y), line)
        scaled = scaled.replace('(at 0 0)', '(at {} {})'.format(x - 3, y - 2)).replace('NAME', 'U{}'.format(unit))
        outp.write(scaled)
    else:
      outp.write(line)

# coding=utf8

import sys
import random

template = \
"""  - name: test%d
    type: test
    pos: [%.1f, %.1f, %.1f]
    scale: %.1f
    rotate: [%.1f, %.1f, %.1f]"""

pos_range = 3
scale_range = 1
rot_range = (0, 360)

for i in xrange(int(sys.argv[1])):
	x = (random.random() - 0.5) * pos_range
	y = (random.random() - 0.5) * pos_range
	z = (random.random() - 0.5) * pos_range
	s = (random.random() - 0.5) * scale_range
	yaw = random.randint(*rot_range)
	pitch = random.randint(*rot_range)
	roll = random.randint(*rot_range)
	print template % (i, x, y, z, s, yaw, pitch, roll)
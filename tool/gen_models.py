# coding=utf8

import sys
import random

template = \
"""  - name: test%d
    type: test
    pos: [%.2f, %.2f, %.2f]
    scale: %.1f
    rotate: [%.1f, %.1f, %.1f]"""

x_range = (-2.0, 2.0)
y_range = (-1.0, 1.0)
z_range = (-10.0, 0.0)
scale_range = (0.4, 0.8)
rot_range = (0, 360)

for i in xrange(int(sys.argv[1])):
	x = random.random() * (x_range[1] - x_range[0]) + x_range[0]
	y = random.random() * (y_range[1] - y_range[0]) + y_range[0]
	z = random.random() * (z_range[1] - z_range[0]) + z_range[0]
	s = random.random() * (scale_range[1] - scale_range[0]) + scale_range[0]
	yaw = random.randint(*rot_range)
	pitch = random.randint(*rot_range)
	roll = random.randint(*rot_range)
	print template % (i, x, y, z, s, yaw, pitch, roll)
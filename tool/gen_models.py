# coding=utf8

import sys
import random

def gen_random(num):
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

	for i in xrange(num):
		x = random.random() * (x_range[1] - x_range[0]) + x_range[0]
		y = random.random() * (y_range[1] - y_range[0]) + y_range[0]
		z = random.random() * (z_range[1] - z_range[0]) + z_range[0]
		s = random.random() * (scale_range[1] - scale_range[0]) + scale_range[0]
		yaw = random.randint(*rot_range)
		pitch = random.randint(*rot_range)
		roll = random.randint(*rot_range)
		print template % (i, x, y, z, s, yaw, pitch, roll)

def gen_step(num):
	template = \
"""  - name: ball%dx%d
    type: balld
    pos: [%.1f, %.1f, 0.0]
    scale: 0.06
    vars:
      material.metallic: %.1f
      material.roughness: %.1f"""

	x_range = (-1.0, 1.0)
	y_range = (-1.0, 1.0)
	pbr = (0.0, 1.0)
	
	content = []
	for i in xrange(num):
		x = x_range[0] + (x_range[1] - x_range[0]) * (i + 1) / num
		m = pbr[0] + (pbr[1] - pbr[0]) * (i + 1) / num
		for j in xrange(num):
			y = y_range[0] + (y_range[1] - y_range[0]) * (j + 1) / num
			r = pbr[0] + (pbr[1] - pbr[0]) * (j + 1) / num
			content.append(template % (i, j, x, y, m, r))
	with open('out', 'wb') as f:
		f.write('\n'.join(content) + '\n')

method = sys.argv[1]
num = int(sys.argv[2])
globals()['gen_' + method](num)
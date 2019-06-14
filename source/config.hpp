#pragma once

#include <vector>

#define PI 3.1415926535897f

#define FPS 60
#define FOV 45
#define WIDTH 1024
#define HEIGHT 768
#define BG_COLOR 0.2f, 0.3f, 0.3f, 1.0f

#define CAM_FOV 1.0f
#define CAM_MOVE 1.0f
#define CAM_ROTATE 0.0005f

#define POS_LOC 0
#define UV_LOC 1
#define NORMAL_LOC 2
#define POS_NAME "vt_pos"
#define UV_NAME "vt_uv"
#define NORMAL_NAME "vt_normal"

// x, y, uv
#define POST_VERTEX           \
	-1.0f, 1.0f, 0.0f, 1.0f,  \
	-1.0f, -1.0f, 0.0f, 0.0f, \
	1.0f, -1.0f, 1.0f, 0.0f,  \
	-1.0f, 1.0f, 0.0f, 1.0f,  \
	1.0f, -1.0f, 1.0f, 0.0f,  \
	1.0f, 1.0f, 1.0f, 1.0f
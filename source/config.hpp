#pragma once

#include <vector>
#include <map>
#include <string>
#include "glad/glad.h"

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
#define POS_NAME "pos"
#define UV_NAME "uv"
#define NORMAL_NAME "normal"

#define VERTEX_BASE "Vertex"
#define VERTEX_INSTANCE "Instance"

#define UNIFORM_MATVP "MatrixVP"
#define UNIFORM_SCENE "Scene"
#define UNIFORM_LIGHTS "Lights"

#define LIGHT_MAX 10
#define LIGHT_DIR 1
#define LIGHT_POINT 2
#define LIGHT_SPOT 3

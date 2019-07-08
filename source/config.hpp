#pragma once

#include <vector>
#include <map>
#include <string>
#include "glad/glad.h"

#define PI 3.1415926535897f

#define CAM_FOV 1.0f
#define CAM_MOVE 1.0f
#define CAM_ROTATE 0.0005f

#define VERTEX_BASE "Vertex"
#define VERTEX_INSTANCE "Instance"

#define UNIFORM_MATVP "MatrixVP"
#define UNIFORM_SCENE "Scene"
#define UNIFORM_LIGHTS "Lights"

#define LIGHT_MAX 10
#define LIGHT_DIR 1
#define LIGHT_POINT 2
#define LIGHT_SPOT 3

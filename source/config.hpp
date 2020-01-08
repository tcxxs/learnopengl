#pragma once

#include <vector>
#include <map>
#include <string>
#include "glad/glad.h"

#define PI 3.1415926535897f

#define PROJ_NEAR 0.1f
#define PROJ_FAR 100.0f

#define CAM_FOV 1.0f
#define CAM_MOVE 2.0f
#define CAM_ROTATE 0.0005f

#define VERTEX_BASE "Vertex"
#define VERTEX_INSTANCE "Instance"

#define UNIFORM_MATVP "MatrixVP"
#define UNIFORM_SCENE "Scene"
#define UNIFORM_LIGHTS "Lights"

#define LIGHT_MAX 10

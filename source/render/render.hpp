#pragma once

#include <string>
#include <filesystem>

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "GLFW/glfw3.h"
#include "config.hpp"
#include "utils/resource.hpp"
#include "render/model.hpp"
#include "render/shader.hpp"
#include "render/camera.hpp"
#include "render/light.hpp"

class Render: public NoCopy {
public:
	bool init();
    void onRender();
private:
	bool _initMaterials();
};

using RenderMgr = Singleton<Render>;
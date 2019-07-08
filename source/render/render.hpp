#pragma once

#include <string>
#include <filesystem>

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "GLFW/glfw3.h"
#include "config.hpp"
#include "utils/resource.hpp"

class Render: public NoCopy {
public:
	bool init();
    void onRender();
};

using RenderMgr = Singleton<Render>;
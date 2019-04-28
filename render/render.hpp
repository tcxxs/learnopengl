#pragma once

#include <string>
#include <filesystem>

#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "config.hpp"
#include "event.hpp"
#include "utils/resource.hpp"
#include "render/model.hpp"
#include "render/shader.hpp"

void oglFeature();

class Render: public NoCopy {
public:
	void init();
    void onRender();

private:
    glm::mat4 _view{1.0f}, _proj{1.0f};
};

using RenderMgr = Singleton<Render>;
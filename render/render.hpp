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
    Render() {
        _view = glm::translate(_view, glm::vec3(0.0f, 0.0f, -3.0f));
        _proj = glm::perspective(glm::radians(45.0f), WIDTH / HEIGHT, 0.1f, 100.0f);
    }

    void onRender();

private:
    glm::mat4 _view{1.0f}, _proj{1.0f};
};

using RenderMgr = Singleton<Render>;
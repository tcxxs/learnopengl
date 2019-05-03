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

class Render: public NoCopy {
public:
	void init();
    void onRender();

    inline const Camera::ptr& getCamera() const { return _cam; }

private:
    Camera::ptr _cam;
};

using RenderMgr = Singleton<Render>;
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
	static void APIENTRY onError(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
	void onRender();

private:
	bool _initVertex();
	bool _initFeature();
};

using RenderMgr = Singleton<Render>;
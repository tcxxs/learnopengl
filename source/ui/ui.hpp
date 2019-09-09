#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "imgui/imgui.h"
#include "utils/resource.hpp"
#include "utils/utils.hpp"

class UI: public NoCopy {
public:
	virtual ~UI();

	bool init(GLFWwindow* window);
	bool onRender();

private:
	bool _renderLog();
	bool _renderScene();

private:
	std::map<std::string, ImFont*> _fonts;
};

using UIMgr = Singleton<UI>;
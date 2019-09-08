#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "imgui/imgui.h"
#include "utils/resource.hpp"
#include "utils/utils.hpp"

class UI: public NoCopy {
public:
	bool init(GLFWwindow* window);
	bool onRender();

	virtual ~UI();
private:
	bool _renderLog(); 

private:
	std::map<std::string, ImFont*> _fonts;
};

using UIMgr = Singleton<UI>;
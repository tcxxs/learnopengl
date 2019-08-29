#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "imgui/imgui.h"
#include "utils/resource.hpp"
#include "utils/utils.hpp"

class UI: public NoCopy {
public:
	static bool init(GLFWwindow* window);
	static bool render();

	virtual ~UI();

private:
	inline static std::map<std::string, ImFont*> _fonts;
};

using UIMgr = Singleton<UI>;
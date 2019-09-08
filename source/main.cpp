#include <iostream>

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "GLFW/glfw3.h"
#include "config.hpp"
#include "system.hpp"
#include "utils/resource.hpp"
#include "render/render.hpp"
#include "render/mesh.hpp"
#include "render/model.hpp"
#include "render/scene.hpp"
#include "ui/ui.hpp"

// TODO: 一个测试用界面
// TODO: 可重载程序而不重启
// TODO: shader include/marcro, hlsl > spirv > glsl

int main() {
	if (!SystemMgr::inst().init()) {
		return -1;
	}
	if (!RenderMgr::inst().init()) {
		return -1;
	}
	if (!UIMgr::inst().init(SystemMgr::inst().getWindow())) {
		return -1;
	}

	//const std::string& name = SystemMgr::inst().getScene();
	//const Scene::ptr& scene = SceneMgr::inst().create(name);
	//if (!scene)
	//	return -1;
	//scene->active();

	SystemMgr::inst().process();
	return 0;
}
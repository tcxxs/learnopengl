#include <iostream>

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "GLFW/glfw3.h"
#include "config.hpp"
#include "event.hpp"
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
	if (!EventMgr::inst().init()) {
		return -1;
	}
	if (!UIMgr::inst().init(EventMgr::inst().getWindow())) {
		return -1;
	}
	if (!RenderMgr::inst().init()) {
		return -1;
	}

	const std::string& name = EventMgr::inst().getScene();
	const Scene::ptr& scene = SceneMgr::inst().create(name);
	if (!scene)
		return -1;
	scene->active();

	EventMgr::inst().process();
	return 0;
}
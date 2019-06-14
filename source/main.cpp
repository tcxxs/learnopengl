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

int main() {
	if (!EventMgr::inst().init(FPS)) {
		return -1;
	}
	if (!RenderMgr::inst().init()) {
		return -1;
	}

	const Scene::ptr& scene = SceneMgr::inst().create("scene");
	if (!scene)
		return -1;
	scene->active();

	EventMgr::inst().process();
	return 0;
}
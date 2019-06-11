#include "render.hpp"
#include "render/scene.hpp"
#include "render/material.hpp"

bool Render::init() {
	glViewport(0, 0, WIDTH, HEIGHT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);

	if (!_initMaterials())
		return false;

	oglFeature();
	return true;
}

bool Render::_initMaterials() {
	std::filesystem::path path = std::filesystem::current_path() / "resource" / "materials.yml";
	Config conf;
	if (!conf.load(path)) {
		std::cout << "materials config error";
		return false;
	}

	for (const auto& it: conf.root()) {
		Material::ptr mate = MaterialMgr::inst().req(it.first.as<std::string>(), it.second);
		if (!mate)
			return false;
	}
}

void Render::onRender() {
	glClearColor(BG_COLOR);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	if (Scene::current) {
		Scene::current->draw();
	}
}

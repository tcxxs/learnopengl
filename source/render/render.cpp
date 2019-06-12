#include "render.hpp"
#include "render/scene.hpp"
#include "render/material.hpp"

bool Render::init() {
	glViewport(0, 0, WIDTH, HEIGHT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);

	oglFeature();
	return true;
}

void Render::onRender() {
	glClearColor(BG_COLOR);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	if (Scene::current) {
		Scene::current->draw();
	}
}

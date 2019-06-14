#include "render.hpp"
#include "render/scene.hpp"
#include "render/material.hpp"

bool Render::init() {
	oglFeature();
	return true;
}

void Render::onRender() {
	if (Scene::current) {
		Scene::current->draw();
	}
}

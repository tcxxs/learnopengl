#include "render.hpp"
#include "render/scene.hpp"
#include "render/material.hpp"
#include "render/vertex.hpp"

bool Render::init() {
	glEnable(GL_MULTISAMPLE);
	if (!VertexProtoMgr::inst().req(VERTEX_BASE))
		return false;
	if (!VertexProtoMgr::inst().req(VERTEX_INSTANCE))
		return false;

	oglFeature();
	return true;
}

void Render::onRender() {
	if (Scene::current) {
		Scene::current->draw();
	}
}

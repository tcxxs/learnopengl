#include "render.hpp"
#include "render/scene.hpp"
#include "render/material.hpp"
#include "render/vertex.hpp"

bool Render::init() {
	if (SystemMgr::inst().getDebug()) {
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(oglDebug, nullptr);
		//glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
	}
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	if (SystemMgr::inst().getMSAA())
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

#include "render.hpp"

void Render::init() {
	glViewport(0, 0, WIDTH, HEIGHT);
	glEnable(GL_DEPTH_TEST);

	_cam = Camera::create();
	_cam->setFov(FOV);
	_cam->lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0, 0.0, 0.0));

	oglFeature();
}

void Render::onRender()
{
	glClearColor(BG_COLOR);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (auto& it: ModelMgr::inst().container()) {
		it.second->draw(_cam->getView(), _cam->getProj());
	}
}

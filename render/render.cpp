#include "render.hpp"

Render::~Render() {
	_cam = nullptr;
	_light = nullptr;
}

void Render::init() {
	glViewport(0, 0, WIDTH, HEIGHT);
	glEnable(GL_DEPTH_TEST);

	_cam = Camera::create();
	_cam->setFov(FOV);
	_cam->lookAt(glm::vec3(2.0f, 1.0f, 5.0f), glm::vec3(0.0, 0.0, 0.0));

	_light = Light::create();
	_light->setPos(glm::vec3(1.5f, 1.5f, 1.0f));
	_light->setColor(glm::vec3(1.0f, 0.8f, 0.8f));

	oglFeature();
}

void Render::onRender()
{
	glClearColor(BG_COLOR);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ModelMgr::inst().get("test")->setVar("camera_pos", _cam->getPos());
	for (auto& it: ModelMgr::inst().container()) {
		it.second->draw(_cam->getView(), _cam->getProj());
	}
}

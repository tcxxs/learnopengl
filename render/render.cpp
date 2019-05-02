#include "render.hpp"

void Render::init() {
	glViewport(0, 0, WIDTH, HEIGHT);
	glEnable(GL_DEPTH_TEST);

    _proj = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
	_cam = Camera::create();
	_cam->lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0, 0.0, 0.0));

	oglFeature();
}

void Render::onRender()
{
	glClearColor(BG_COLOR);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	std::cout << _cam->getYaw() << "," << _cam->getPitch() << std::endl;
	for (auto& it: ModelMgr::inst().container()) {
		glm::mat4 model(1.0f);
		model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
		//it.second->matrixModel(model);
		it.second->draw(_cam->getMatrix(), _proj);
	}
}

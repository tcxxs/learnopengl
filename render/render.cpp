#include "render.hpp"

void oglFeature() {
	int nrAttributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	std::cout << "Maximum nr of vertex attributes supported: " << nrAttributes
	          << std::endl;
}

void Render::init() {
	glViewport(0, 0, WIDTH, HEIGHT);
	glEnable(GL_DEPTH_TEST);
    _view = glm::translate(_view, glm::vec3(0.0f, 0.0f, -3.0f));
    _proj = glm::perspective(glm::radians(45.0f), WIDTH / HEIGHT, 0.1f, 100.0f);

	oglFeature();
}

void Render::onRender()
{
	glClearColor(BG_COLOR);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (auto& it: ModelMgr::inst().container()) {
		glm::mat4 model(1.0f);
		model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
		it.second->matrixModel(model);
		it.second->draw(_view, _proj);
	}

}

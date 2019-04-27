#include "render.hpp"

void oglFeature() {
	int nrAttributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	std::cout << "Maximum nr of vertex attributes supported: " << nrAttributes
	          << std::endl;
}

void Render::onRender()
{
	glClearColor(BG_COLOR);
	glClear(GL_COLOR_BUFFER_BIT);

	for (auto& it: ModelMgr::inst().container()) {
		it.second->draw(_view, _proj);
	}

}

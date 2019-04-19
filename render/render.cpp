#include "render.hpp"

void onRender()
{
	glClearColor(BG_COLOR);
	glClear(GL_COLOR_BUFFER_BIT);

	ShaderMgr::inst().get("simple")->useProgram();
	for (auto& it: MeshMgr::inst().container()) {
		it.second->draw();
	}

}

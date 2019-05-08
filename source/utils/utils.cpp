#include "utils.hpp"

void oglFeature() {
	int nrAttributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	std::cout << "Maximum nr of vertex attributes supported: " << nrAttributes
	          << std::endl;
}

bool oglError() {
	bool ret = false;
	GLenum err;
	while((err = glGetError()) != GL_NO_ERROR)
	{
		ret = true;
		std::cout << "opengl error: " << err <<std::endl;
	}

	return ret;
}

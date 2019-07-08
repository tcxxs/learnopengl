#pragma once

#include "glad/glad.h"
#include "config.hpp"
#include "utils/resource.hpp"

class Frame: public Res<Frame> {
public:
	static ptr create();

	virtual ~Frame();

	bool attachTexture();
	bool attachDepthStencil();
	bool attachRenderBuffer();

	GLuint getTexture();

	bool useBegin();
	inline void useEnd() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

private:
	GLuint _fbo{0};
	GLuint _rbo{0}, _tex{0}, _ds{0};
	GLuint _fboblit{0}, _texblit{0};
};

#pragma once

#include "glad/glad.h"
#include "config.hpp"
#include "utils/resource.hpp"
#include "render/texture.hpp"

class Frame: public Res<Frame> {
public:
	static ptr create();

	virtual ~Frame();

	bool attachTexture();
	bool attachDepthStencil();
	bool attachRenderBuffer();
	bool attachShadowMap();
	bool checkStatus();

	inline GLuint getFBO() const { return _fbo; }
	const Texture::val getTexture();

	inline void setDirty() { _blitdirty = true; }

private:
	GLuint _fbo{0};
	GLuint _rbo{0}, _tex{0}, _ds{0};
	GLbitfield _blittype{0};
	bool _blitdirty{true};
	GLuint _fboblit{0}, _texblit{0};
};

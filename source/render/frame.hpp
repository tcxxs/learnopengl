#pragma once

#include "glad/glad.h"
#include "config.hpp"
#include "event.hpp"
#include "utils/resource.hpp"
#include "render/texture.hpp"

class Frame: public Res<Frame> {
public:
	static ptr create(const Config::node& conf);

	virtual ~Frame();

	inline std::pair<int, int> getView() const {
		int width = EventMgr::inst().getWidth();
		int height = EventMgr::inst().getHeight();
		if (_textype == GL_TEXTURE_CUBE_MAP) {
			int size = int(std::max(width, height) * _size);
			return {size, size};
		}
		else {
			return {int(width * _size), int(height * _size)};
		}
	}
	inline GLuint getFBO() const { return _fbo; }
	const Texture::val getTexture();

	inline void setDirty() { _blitdirty = true; }

private:
	bool _checkStatus();
	bool _attachTexture();
	bool _attachDepthStencil();
	bool _attachRenderBuffer();
	bool _attachDepth();
	bool _attachDepthCube();

private:
	float _size{0.0f};
	GLuint _fbo{0};
	GLuint _rbo{0}, _tex{0}, _ds{0};
	GLbitfield _textype{0}, _blittype{0};
	bool _blitdirty{true};
	GLuint _fboblit{0}, _texblit{0};
};

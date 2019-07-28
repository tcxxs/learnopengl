#pragma once

#include "glad/glad.h"
#include "config.hpp"
#include "event.hpp"
#include "utils/resource.hpp"
#include "render/texture.hpp"

struct Attachment {
	int index{0};
	std::string name;
	GLenum type{0};
	GLuint tex{0};
	GLuint blit_fbo{0};
	GLuint blit_tex{0};
	bool blit_dirty{false};
};

class Frame: public Res<Frame> {
public:
	static ptr create(const Config::node& conf);

	virtual ~Frame();
	void deleteAttach(Attachment& attach);

	inline std::pair<int, int> getView() const {
		int width = int(EventMgr::inst().getWidth() * _size);
		int height = int(EventMgr::inst().getHeight() * _size);
		if (_square) {
			width = std::max(width, height);
			height = width;
		}

		return {width, height};
	}

	inline GLuint getFBO() const { return _fbo; }
	inline const Texture::val getTexture(const std::string& name) {
		Attachment& attach = _getAttach(name);
		return _getTexture(attach);
	}
	inline void setDirty(const std::string& name) {
		Attachment& attach = _getAttach(name);
		_setDirty(attach);
	}
	inline void setDirtyAll() {
		for (auto& it: _colors)
			_setDirty(it);
		_setDirty(_depth);
		_setDirty(_stencil);
	}

private:
	// TODO: color支持msaa，可以配合depst用；其余可读的depth不支持msaa
	bool _completeFrame();
	bool _attachTexture(Attachment& attach, GLenum format);
	inline bool _attachLDR(Attachment& attach) { return _attachTexture(attach, GL_RGBA); }
	inline bool _attachHDR(Attachment& attach) { return _attachTexture(attach, GL_RGBA16F); }
	bool _attachDepthStencil(Attachment& attach);
	bool _attachDepth(Attachment& attach);
	bool _attachDepthCube(Attachment& attach);

	inline Attachment& _getAttach(const std::string& name) {
		if (name == "depth")
			return _depth;
		else if (name == "stencil")
			return _stencil;
		else if (name == "color") {
			if (_colors.empty())
				return _empty;
			else
				return _colors[0];
		}
		else {
			for (auto& it: _colors) {
				if (it.name == name)
					return it;
			}
		}

		return _empty;
	}
	const Texture::val _getTexture(Attachment& attach);
	inline void _setDirty(Attachment& attach) {
		if (attach.blit_fbo)
			attach.blit_dirty = true;
	}

private:
	inline static Attachment _empty;
	float _size{0.0f};
	bool _square{false};
	GLuint _fbo{0};
	std::vector<Attachment> _colors;
	Attachment _depth;
	Attachment _stencil;
};

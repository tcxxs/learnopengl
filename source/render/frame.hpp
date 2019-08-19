#pragma once

#include "glad/glad.h"
#include "config.hpp"
#include "event.hpp"
#include "utils/resource.hpp"
#include "render/texture.hpp"

class Frame: public Res<Frame> {
public:
	struct Attachment {
		int index{0};
		std::string name;
		GLenum type{0};
		GLuint tex{0};
		GLuint blit_fbo{0};
		GLuint blit_tex{0};
		bool blit_dirty{false};
	};

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
	Attachment& getAttach(const std::string& name);

	const Texture::val getTexture(Attachment& attach);
	inline const Texture::val getTexture(const std::string& name) {
		Attachment& attach = getAttach(name);
		return getTexture(attach);
	}

	inline void setDirty(Attachment& attach) {
		if (attach.blit_fbo)
			attach.blit_dirty = true;
	}
	inline void setDirty(const std::string& name) {
		Attachment& attach = getAttach(name);
		setDirty(attach);
	}
	inline void setDirtyAll() {
		for (auto& it: _colors)
			setDirty(it);
		setDirty(_depth);
		setDirty(_stencil);
	}

private:
	// TODO: color支持msaa，可以配合depst用；其余可读的depth不支持msaa
	bool _completeFrame();
	bool _attachColor(const Config::node& conf);
	bool _attachTexture(Attachment& attach, GLenum format);
	bool _attachCubemap(Attachment& attach, GLenum format);
	bool _attachDepthStencil(Attachment& attach);
	bool _attachDepth(Attachment& attach);
	bool _attachDepthCube(Attachment& attach);

private:
	inline static Attachment _empty;
	float _size{0.0f};
	bool _square{false};
	GLuint _fbo{0};
	std::vector<Attachment> _colors;
	Attachment _depth;
	Attachment _stencil;
};

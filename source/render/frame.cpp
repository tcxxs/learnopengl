#include "frame.hpp"
#include "utils/utils.hpp"

Frame::ptr Frame::create(const Config::node& conf) {
	Frame::ptr frame = std::make_shared<Frame>();

	glGenFramebuffers(1, &frame->_fbo);
	if (EventMgr::inst().getMSAA() > 0) {
		glGenFramebuffers(1, &frame->_fboblit);
	}

	frame->_size = conf["size"].as<float>(1.0f);
	for (const auto& it: conf["attach"]) {
		const std::string& attach = it.as<std::string>();
		if (attach == "texture")
			frame->_attachTexture();
		else if (attach == "hdr")
			frame->_attachHDR();
		else if (attach == "depst")
			frame->_attachDepthStencil();
		else if (attach == "depth")
			frame->_attachDepth();
		else if (attach == "depcube")
			frame->_attachDepthCube();
	}
	if (!frame->_checkStatus())
		return {};

	return frame;
}

Frame::~Frame() {
	if (_fbo) {
		glDeleteFramebuffers(1, &_fbo);
	}
	if (_fboblit) {
		glDeleteFramebuffers(1, &_fboblit);
	}
	if (_tex) {
		glDeleteTextures(1, &_tex);
	}
	if (_texblit) {
		glDeleteTextures(1, &_texblit);
	}
	if (_ds) {
		glDeleteRenderbuffers(1, &_ds);
	}
	if (_rbo) {
		glDeleteRenderbuffers(1, &_rbo);
	}
}

bool Frame::_checkStatus() {
	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	GLenum ret = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if (ret != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "frame use, not complete" << std::endl;
		return false;
	}

	return true;
}

bool Frame::_attachTexture() {
	int msaa = EventMgr::inst().getMSAA();
	int width = int(EventMgr::inst().getWidth() * _size);
	int height = int(EventMgr::inst().getHeight() * _size);

	_textype = GL_TEXTURE_2D;
	GLenum type;
	if (msaa > 0) {
		type = GL_TEXTURE_2D_MULTISAMPLE;
	}
	else {
		type = GL_TEXTURE_2D;
	}
	glGenTextures(1, &_tex);
	glBindTexture(type, _tex);
	if (msaa > 0) {
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, msaa, GL_RGBA, width, height, GL_TRUE);
	}
	else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, type, _tex, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(type, 0);

	if (msaa > 0) {
		_blittype = GL_COLOR_BUFFER_BIT;
		glGenTextures(1, &_texblit);
		glBindTexture(GL_TEXTURE_2D, _texblit);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glBindFramebuffer(GL_FRAMEBUFFER, _fboblit);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texblit, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	return oglError();
}

bool Frame::_attachHDR() {
	int msaa = EventMgr::inst().getMSAA();
	int width = int(EventMgr::inst().getWidth() * _size);
	int height = int(EventMgr::inst().getHeight() * _size);

	_textype = GL_TEXTURE_2D;
	GLenum type;
	if (msaa > 0) {
		type = GL_TEXTURE_2D_MULTISAMPLE;
	}
	else {
		type = GL_TEXTURE_2D;
	}
	glGenTextures(1, &_tex);
	glBindTexture(type, _tex);
	if (msaa > 0) {
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, msaa, GL_RGBA16F, width, height, GL_TRUE);
	}
	else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, type, _tex, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(type, 0);

	if (msaa > 0) {
		_blittype = GL_COLOR_BUFFER_BIT;
		glGenTextures(1, &_texblit);
		glBindTexture(GL_TEXTURE_2D, _texblit);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glBindFramebuffer(GL_FRAMEBUFFER, _fboblit);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texblit, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	return oglError();
}

bool Frame::_attachDepthStencil() {
	int msaa = EventMgr::inst().getMSAA();
	int width = int(EventMgr::inst().getWidth() * _size);
	int height = int(EventMgr::inst().getHeight() * _size);

	//glGenTextures(1, &_ds);
	//glBindTexture(GL_TEXTURE_2D, _ds);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);

	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, _ds, 0);
	//glBindTexture(GL_TEXTURE_2D, 0);

	glGenRenderbuffers(1, &_rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, _rbo);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, msaa, GL_DEPTH24_STENCIL8, width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbo);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	return oglError();
}

bool Frame::_attachRenderBuffer() {
	int msaa = EventMgr::inst().getMSAA();
	int width = int(EventMgr::inst().getWidth() * _size);
	int height = int(EventMgr::inst().getHeight() * _size);

	glGenRenderbuffers(1, &_rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, _rbo);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, msaa, GL_RGBA, width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _rbo);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	return oglError();
}

bool Frame::_attachDepth() {
	int width = int(EventMgr::inst().getWidth() * _size);
	int height = int(EventMgr::inst().getHeight() * _size);

	_textype = GL_TEXTURE_2D;
	glGenTextures(1, &_tex);
	glBindTexture(GL_TEXTURE_2D, _tex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	GLfloat border[] = {1.0, 1.0, 1.0, 1.0};
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _tex, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	return oglError();
}

bool Frame::_attachDepthCube() {
	int width = int(EventMgr::inst().getWidth() * _size);
	int height = int(EventMgr::inst().getHeight() * _size);
	int size = std::max(width, height);

	_textype = GL_TEXTURE_CUBE_MAP;
	glGenTextures(1, &_tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _tex);

	for (GLuint i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, size, size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _tex, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return oglError();
}

const Texture::val Frame::getTexture()
{
	int msaa = EventMgr::inst().getMSAA();
	int width = int(EventMgr::inst().getWidth() * _size);
	int height = int(EventMgr::inst().getHeight() * _size);

	if (msaa && _blittype) {
		if (_blitdirty) {
			_blitdirty = false;
			glBindFramebuffer(GL_READ_FRAMEBUFFER, _fbo);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fboblit);
			glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, _blittype, GL_NEAREST);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		}
		return {_texblit, _textype};
	}
	else {
		return {_tex, _textype};
	}
}

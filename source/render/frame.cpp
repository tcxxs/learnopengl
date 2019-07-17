#include "frame.hpp"
#include "event.hpp"
#include "utils/utils.hpp"

Frame::ptr Frame::create(const Config::node& conf) {
	Frame::ptr frame = std::shared_ptr<Frame>(new Frame());

	glGenFramebuffers(1, &frame->_fbo);
	if (EventMgr::inst().getMSAA() > 0) {
		glGenFramebuffers(1, &frame->_fboblit);
	}

	frame->_size = conf["size"].as<float>(1.0f);
	for (const auto& it: conf["attach"]) {
		const std::string& attach = it.as<std::string>();
		if (attach == "texture")
			frame->_attachTexture();
		else if (attach == "depst")
			frame->_attachDepthStencil();
		else if (attach == "shadow")
			frame->_attachShadowMap();
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

bool Frame::_attachTexture() {
	int msaa = EventMgr::inst().getMSAA();
	int width = int(EventMgr::inst().getWidth() * _size);
	int height = int(EventMgr::inst().getHeight() * _size);

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
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
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
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
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
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

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

bool Frame::_attachShadowMap() {
	int msaa = EventMgr::inst().getMSAA();
	int width = int(EventMgr::inst().getWidth() * _size);
	int height = int(EventMgr::inst().getHeight() * _size);

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
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, msaa, GL_DEPTH_COMPONENT, width, height, GL_TRUE);
	}
	else {
		// opengl 4.6，TEXTURE_2D_MULTISAMPLE初始化这几个属性会报错
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		GLfloat border[] = {1.0, 1.0, 1.0, 1.0};
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, type, _tex, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(type, 0);

	if (msaa > 0) {
		_blittype = GL_DEPTH_BUFFER_BIT;
		glGenTextures(1, &_texblit);
		glBindTexture(GL_TEXTURE_2D, _texblit);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		GLfloat border[] = {1.0, 1.0, 1.0, 1.0};
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		glBindFramebuffer(GL_FRAMEBUFFER, _fboblit);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _texblit, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	return oglError();
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

const Texture::val Frame::getTexture()
{
	int msaa = EventMgr::inst().getMSAA();
	int width = int(EventMgr::inst().getWidth() * _size);
	int height = int(EventMgr::inst().getHeight() * _size);

	if (msaa > 0) {
		if (_blitdirty) {
			_blitdirty = false;
			glBindFramebuffer(GL_READ_FRAMEBUFFER, _fbo);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fboblit);
			glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, _blittype, GL_NEAREST);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		}
		return {_texblit};
	}
	else {
		return {_tex};
	}
}

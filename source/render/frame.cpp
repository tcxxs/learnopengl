#include "frame.hpp"
#include "event.hpp"
#include "utils/utils.hpp"

Frame::ptr Frame::create() {
	Frame::ptr frame = std::shared_ptr<Frame>(new Frame());

	glGenFramebuffers(1, &frame->_fbo);
	if (EventMgr::inst().getMSAA() > 0) {
		glGenFramebuffers(1, &frame->_fboblit);
	}
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

bool Frame::attachTexture() {
	int msaa = EventMgr::inst().getMSAA();
	int width = EventMgr::inst().getWidth();
	int height = EventMgr::inst().getHeight();

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
	}
	glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, type, _tex, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(type, 0);

	if (msaa > 0) {
		glGenTextures(1, &_texblit);
		glBindTexture(GL_TEXTURE_2D, _texblit);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindFramebuffer(GL_FRAMEBUFFER, _fboblit);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texblit, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	return oglError();
}

bool Frame::attachDepthStencil() {
	int msaa = EventMgr::inst().getMSAA();
	int width = EventMgr::inst().getWidth();
	int height = EventMgr::inst().getHeight();

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

bool Frame::attachRenderBuffer() {
	int msaa = EventMgr::inst().getMSAA();
	int width = EventMgr::inst().getWidth();
	int height = EventMgr::inst().getHeight();

	glGenRenderbuffers(1, &_rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, _rbo);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, msaa, GL_RGBA, width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _rbo);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	return oglError();
}

bool Frame::attachShadowMap() {
	int width = EventMgr::inst().getWidth();
	int height = EventMgr::inst().getHeight();

	glGenTextures(1, &_tex);
	glBindTexture(GL_TEXTURE_2D, _tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _tex, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

GLuint Frame::getTexture() {
	int msaa = EventMgr::inst().getMSAA();
	int width = EventMgr::inst().getWidth();
	int height = EventMgr::inst().getHeight();

	if (msaa > 0) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, _fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fboblit);
		glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		return _texblit;
	}
	else {
		return _tex;
	}
}

bool Frame::drawBegin() {
	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "frame use, not complete" << std::endl;
		return false;
	}
	return true;
}
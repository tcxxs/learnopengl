#include "frame.hpp"
#include "utils/utils.hpp"

Frame::ptr Frame::create() {
	Frame::ptr frame = std::shared_ptr<Frame>(new Frame());

	glGenFramebuffers(1, &frame->_fbo);
	return frame;
}

Frame::~Frame() {
	if (_fbo) {
		glDeleteFramebuffers(1, &_fbo);
	}
	if (_tex) {
		glDeleteTextures(1, &_tex);
	}
	if (_ds) {
		glDeleteRenderbuffers(1, &_ds);
	}
	if (_rbo) {
		glDeleteRenderbuffers(1, &_rbo);
	}
}

bool Frame::attachTexture() {
	glGenTextures(1, &_tex);
	glBindTexture(GL_TEXTURE_2D, _tex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _tex, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	return oglError();
}

bool Frame::attachDepthStencil() {
	//glGenTextures(1, &_ds);
	//glBindTexture(GL_TEXTURE_2D, _ds);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, _ds, 0);
	//glBindTexture(GL_TEXTURE_2D, 0);

	glGenRenderbuffers(1, &_ds);
	glBindRenderbuffer(GL_RENDERBUFFER, _ds);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);

	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _ds);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	return oglError();
}

bool Frame::attachRenderBuffer() {
	glGenRenderbuffers(1, &_rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, _rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);

	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbo);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	return oglError();
}

bool Frame::useBegin() {
	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "frame use, not complete" << std::endl;
		return false;
	}
	return true;
}
#include "frame.hpp"
#include "utils/utils.hpp"

Frame::ptr Frame::create(const Config::node& conf) {
	Frame::ptr frame = std::make_shared<Frame>();

	glGenFramebuffers(1, &frame->_fbo);
	frame->_size = conf["size"].as<float>(1.0f);
	frame->_square = conf["square"].as<bool>(false);

	std::string type;
	if (Config::valid(conf["colors"])) {
		for (const auto& it: conf["colors"]) {
			Attachment& attach = frame->_colors.emplace_back();
			attach.index = (int)frame->_colors.size() - 1;
			if (it.IsMap()) {
				type = it.begin()->first.as<std::string>();
				attach.name = it.begin()->second.as<std::string>();
			}
			else {
				type = it.as<std::string>();
			}
			if (type == "ldr")
				frame->_attachTexture(attach, GL_RGBA);
			else if (type == "hdr")
				frame->_attachTexture(attach, GL_RGBA16F);
			else if (type == "rf")
				frame->_attachTexture(attach, GL_R16F);
			else if (type == "cube")
				frame->_attachTextureCube(attach, GL_RGBA);
			else if (type == "cubef")
				frame->_attachTextureCube(attach, GL_RGBA16F);
		}
	}

	if (Config::valid(conf["depth"])) {
		const std::string& type = conf["depth"].as<std::string>();
		if (type == "depst")
			frame->_attachDepthStencil(frame->_depth);
		else if (type == "depth")
			frame->_attachDepth(frame->_depth);
		else if (type == "depcube")
			frame->_attachDepthCube(frame->_depth);
	}

	if (!frame->_completeFrame())
		return {};

	return frame;
}

Frame::~Frame() {
	for (auto& it: _colors) {
		deleteAttach(it);
	}
	deleteAttach(_depth);
	deleteAttach(_stencil);

	if (_fbo) {
		glDeleteFramebuffers(1, &_fbo);
	}
}

void Frame::deleteAttach(Attachment& attach) {
	if (attach.tex) {
		if (attach.type == GL_RENDERBUFFER)
			glDeleteRenderbuffers(1, &attach.tex);
		else
			glDeleteTextures(1, &attach.tex);
		attach.tex = 0;
	}
	if (attach.blit_tex) {
		glDeleteTextures(1, &attach.blit_tex);
		attach.blit_tex = 0;
	}
	if (attach.blit_fbo) {
		glDeleteFramebuffers(1, &attach.blit_fbo);
		attach.blit_fbo = 0;
	}
}

bool Frame::_completeFrame() {
	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	if (_colors.empty()) {
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	GLenum ret = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (ret != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "frame use, not complete" << std::endl;
		return false;
	}
	return true;
}

bool Frame::_attachTexture(Attachment& attach, GLenum format) {
	int msaa = EventMgr::inst().getMSAA();
	int width = int(EventMgr::inst().getWidth() * _size);
	int height = int(EventMgr::inst().getHeight() * _size);
	if (_square) {
		width = std::max(width, height);
		height = width;
	}
	
	if (msaa > 0)
		attach.type = GL_TEXTURE_2D_MULTISAMPLE;
	else
		attach.type = GL_TEXTURE_2D;

	glGenTextures(1, &attach.tex);
	glBindTexture(attach.type, attach.tex);
	if (msaa > 0) {
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, msaa, format, width, height, GL_TRUE);
	}
	else {
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// TODO: 比如bloom中，可能会因为边缘采样到repeat，产生错误
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attach.index, attach.type, attach.tex, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(attach.type, 0);

	if (msaa > 0) {
		glGenTextures(1, &attach.blit_tex);
		glBindTexture(GL_TEXTURE_2D, attach.blit_tex);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glGenFramebuffers(1, &attach.blit_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, attach.blit_fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, attach.blit_tex, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	return oglError();
}

bool Frame::_attachTextureCube(Attachment& attach, GLenum format) {
	int width = int(EventMgr::inst().getWidth() * _size);
	int height = int(EventMgr::inst().getHeight() * _size);
	if (_square) {
		width = std::max(width, height);
		height = width;
	}

	attach.type = GL_TEXTURE_CUBE_MAP;
	glGenTextures(1, &attach.tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, attach.tex);
	for (GLuint i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attach.index, attach.tex, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return oglError();
}

bool Frame::_attachDepthStencil(Attachment& attach) {
	int msaa = EventMgr::inst().getMSAA();
	int width = int(EventMgr::inst().getWidth() * _size);
	int height = int(EventMgr::inst().getHeight() * _size);
	if (_square) {
		width = std::max(width, height);
		height = width;
	}

	//glGenTextures(1, &_ds);
	//glBindTexture(GL_TEXTURE_2D, _ds);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);

	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, _ds, 0);
	//glBindTexture(GL_TEXTURE_2D, 0);

	attach.type = GL_RENDERBUFFER;
	glGenRenderbuffers(1, &attach.tex);
	glBindRenderbuffer(GL_RENDERBUFFER, attach.tex);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, msaa, GL_DEPTH24_STENCIL8, width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, attach.tex);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	return oglError();
}

bool Frame::_attachDepth(Attachment& attach) {
	int width = int(EventMgr::inst().getWidth() * _size);
	int height = int(EventMgr::inst().getHeight() * _size);
	if (_square) {
		width = std::max(width, height);
		height = width;
	}

	attach.type = GL_TEXTURE_2D;
	glGenTextures(1, &attach.tex);
	glBindTexture(GL_TEXTURE_2D, attach.tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	GLfloat border[] = {1.0, 1.0, 1.0, 1.0};
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, attach.tex, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	return oglError();
}

bool Frame::_attachDepthCube(Attachment& attach) {
	int width = int(EventMgr::inst().getWidth() * _size);
	int height = int(EventMgr::inst().getHeight() * _size);
	if (_square) {
		width = std::max(width, height);
		height = width;
	}

	attach.type = GL_TEXTURE_CUBE_MAP;
	glGenTextures(1, &attach.tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, attach.tex);
	for (GLuint i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, attach.tex, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return oglError();
}

Frame::Attachment& Frame::getAttach(const std::string& name) {
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

const Texture::val Frame::getTexture(Attachment& attach)
{
	if (!attach.type)
		return {0};

	int msaa = EventMgr::inst().getMSAA();
	int width = int(EventMgr::inst().getWidth() * _size);
	int height = int(EventMgr::inst().getHeight() * _size);
	if (_square) {
		width = std::max(width, height);
		height = width;
	}

	if (attach.blit_fbo) {
		if (attach.blit_dirty) {
			attach.blit_dirty = false;
			if (attach.index > 0)
				glNamedFramebufferReadBuffer(_fbo, GL_COLOR_ATTACHMENT0 + attach.index);
			glBlitNamedFramebuffer(_fbo, attach.blit_fbo, 0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
			if (attach.index > 0)
				glNamedFramebufferReadBuffer(_fbo, GL_COLOR_ATTACHMENT0);
		}
		return {attach.blit_tex, GL_TEXTURE_2D};
	}
	else {
		return {attach.tex, attach.type};
	}
}

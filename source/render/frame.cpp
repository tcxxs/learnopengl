#include "frame.hpp"
#include "utils/utils.hpp"

Frame::ptr Frame::create(const Config::node& conf) {
	Frame::ptr frame = std::make_shared<Frame>();

	glGenFramebuffers(1, &frame->_fbo);
	frame->_size = conf["size"].as<float>(1.0f);
	frame->_square = conf["square"].as<bool>(false);

	if (Config::valid(conf["colors"])) {
		for (const auto& it: conf["colors"]) {
			if (!frame->_attachColor(it))
				return {};
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

bool Frame::_attachColor(const Config::node& conf) {
	Attachment& attach = _colors.emplace_back();
	attach.index = (int)_colors.size() - 1;

	Config::node type;
	if (conf.IsMap()) {
		attach.name = conf.begin()->first.as<std::string>();
		type = conf.begin()->second;
	}
	else {
		type = conf;
	}

	if (!type.IsSequence() || type.size() < 2) {
		std::printf("frame type error, line %d", type.Mark().line);
		return false;
	}
	const std::string& base = type[0].as<std::string>();
	const std::string& pixel = type[1].as<std::string>();

	GLenum pf{GL_RGBA};
	if (pixel == "rgba8")
		pf = GL_RGBA;
	else if (pixel == "rgba16f")
		pf = GL_RGBA16F;
	else if (pixel == "r16f")
		pf = GL_R16F;
	else {
		std::printf("frame color pixel format error, %s", pixel.c_str());
		return false;
	}

	if (base == "tex") {
		if (!_attachTexture(attach, pf))
			return false;
	}
	else if (base == "cube") {
		if (!_attachCubemap(attach, pf))
			return false;
	}
	else {
		std::printf("frame base type error, %s", base.c_str());
		return false;
	}

	if (type.size() > 2) {
		attach.mip_auto = type[2].as<std::string>() == "auto";
		attach.mip_index = attach.index;
		glBindTexture(attach.type, attach.tex);
		glGenerateMipmap(attach.type);
		glTexParameteri(attach.type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glBindTexture(attach.type, 0);
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
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
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

	return true;
}

bool Frame::_attachCubemap(Attachment& attach, GLenum format) {
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
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attach.index, attach.tex, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return true;
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

int Frame::_genMipmap(const std::string& name) {
	size_t begin = name.find('[');
	size_t end = name.find(']');
	if (begin == std::string::npos || end == std::string::npos)
		return -1;

	std::string mipname = name.substr(0, begin);
	std::string miplvl = name.substr(begin + 1, end - begin - 1);
	for (auto& it: _colors) {
		if (it.mip_index < 0)
			continue;
		if (it.name != mipname)
			continue;

		int width = int(EventMgr::inst().getWidth() * _size);
		int height = int(EventMgr::inst().getHeight() * _size);
		int max = int(std::floor(std::log2(std::max(width, height))));
		int mip = std::stoi(miplvl);
		if (mip == 0) {
			return it.index;
		}
		else if (mip > max) {
			std::printf("frame color %s, mipmap level too high", name.c_str());
			return -1;
		}

		Attachment attach;
		attach.index = (int)_colors.size();
		attach.name = name;
		attach.type = it.type;
		attach.mip_index = it.index;
		attach.mip_level = mip;

		glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attach.index, it.tex, attach.mip_level);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 在colors内，更改元素，最后返回的时候做，避免it错误
		_colors.push_back(attach);
		return attach.index;
	}

	return -1;
}

void Frame::_cleanDirty(Attachment& attach) {
	if (!attach.dirty)
		return;
	if (attach.mip_level > 0)
		return;

	attach.dirty = false;
	// 重新生成mipmap
	if (attach.mip_auto && attach.mip_index >= 0) {
		glBindTexture(attach.type, attach.tex);
		glGenerateMipmap(attach.type);
		glBindTexture(attach.type, 0);
	}
	// 重新拷贝msaa
	if (attach.blit_fbo) {
		int width = int(EventMgr::inst().getWidth() * _size);
		int height = int(EventMgr::inst().getHeight() * _size);
		if (_square) {
			width = std::max(width, height);
			height = width;
		}

		if (attach.index > 0)
			glNamedFramebufferReadBuffer(_fbo, GL_COLOR_ATTACHMENT0 + attach.index);
		glBlitNamedFramebuffer(_fbo, attach.blit_fbo, 0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		if (attach.index > 0)
			glNamedFramebufferReadBuffer(_fbo, GL_COLOR_ATTACHMENT0);
	}
}

std::pair<int, int> Frame::getView(const std::map<GLuint, std::string>& outs) const {
	int width = int(EventMgr::inst().getWidth() * _size);
	int height = int(EventMgr::inst().getHeight() * _size);
	if (_square) {
		width = std::max(width, height);
		height = width;
	}

	if (_colors.empty())
		return {width, height};
	else {
		int mip = 0;
		for (const auto& it: outs) {
			const Attachment& attach = _colors[it.first];
			if (attach.mip_level > mip)
				mip = attach.mip_level;
		}

		mip = int(std::pow(2, mip));
		return {std::max(1, width / mip), std::max(1, height / mip)};
	}
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

		int index = _genMipmap(name);
		if (index >= 0)
			return _colors[index];
	}

	return _empty;
}

const Texture::val Frame::getTexture(Attachment& attach) {
	if (!attach.type)
		return {0};

	if (attach.mip_level > 0)
		return getTexture(_colors[attach.mip_index]);
	
	_cleanDirty(attach);
	if (attach.blit_fbo)
		return {attach.blit_tex, GL_TEXTURE_2D};
	else
		return {attach.tex, attach.type};
}

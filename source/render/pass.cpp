#include "pass.hpp"
#include "event.hpp"

Pass::ptr Pass::create(const Config::node& conf, const genfunc& gen) {
	Pass::ptr pass = std::shared_ptr<Pass>(new Pass());

	if (!pass->_initConf(conf, gen))
		return {};
	if (!pass->_initShader(conf["shaders"], gen))
		return {};
	if (!pass->_initPost(conf["posts"], gen))
		return {};
	if (!pass->_initState(conf["states"]))
		return {};
	return pass;
}

bool Pass::_initConf(const Config::node& conf, const genfunc& gen) {
	setName(conf["name"].as<std::string>());
	if (Config::valid(conf["camera"])) {
		std::any ret = gen(conf["camera"]);
		if (!ret.has_value()) {
			std::printf("pass %s, camera %s, not valid\n", _name.c_str(), conf["camera"].Scalar().c_str());
			return false;
		}
		_cam = std::any_cast<Camera::ptr>(ret);
	}
	if (Config::valid(conf["out"])) {
		std::any ret = gen(conf["out"]);
		if (!ret.has_value()) {
			std::printf("pass %s, out %s, not valid\n", _name.c_str(), conf["out"].Scalar().c_str());
			return false;
		}
		const auto& frame = std::any_cast<const frameinfo&>(ret);
		_out = frame.second;
	}

	return true;
}

bool Pass::_initShaderAttrs(const Config::node& conf, const genfunc& gen, const Shader::ptr& shader) {
	if (!Config::valid(conf))
		return true;

	auto insert = _sattrs.try_emplace(shader->getName(), ShaderAttrs());
	if (!insert.second) {
		std::printf("pass %s, shader %s, duplicate\n", _name.c_str(), shader->getName().c_str());
		return false;
	}
	ShaderAttrs& attrs = insert.first->second;

	GLint loc;
	for (const auto& it: conf) {
		const std::string& name = it.first.as<std::string>();
		loc = shader->getVar(name);
		if (loc < 0) {
			std::printf("pass %s, shader %s, uniform %s, not found\n", _name.c_str(), shader->getName().c_str(), name.c_str());
			return false;
		}

		if (Config::generator(it.second)) {
			std::any ret = gen(it.second);
			if (!ret.has_value()) {
				std::printf("pass %s, shader %s, uniform %s, value %s, not valid\n", _name.c_str(), shader->getName().c_str(), name.c_str(), it.second.Scalar().c_str());
				return false;
			}
			if (ret.type() == typeid(frameinfo)) {
				const auto& frame = std::any_cast<const frameinfo&>(ret);
				if (frame.first == "in") {
					attrs.ins[name] = frame.second;
				}
				else if (frame.first == "out") {
				}
			}
			else {
				attrs.attrs.setAttr(name, ret);
			}
		}
		else {
			std::any val = Config::guess(it.second);
			if (!val.has_value()) {
				std::printf("pass %s, shader %s, uniform %s, value %s, not valid\n", _name.c_str(), shader->getName().c_str(), name.c_str(), it.second.Scalar().c_str());
				return false;
			}
			attrs.attrs.setAttr(name, val);
		}
	}

	return true;
}

bool Pass::_initShader(const Config::node& conf, const genfunc& gen) {
	if (!Config::valid(conf))
		return true;

	for (const auto& it: conf) {
		const std::string& name = it.first.as<std::string>();
		const Shader::ptr& shader = ShaderMgr::inst().req(name);
		if (!shader) {
			std::cout << "pass shader not found, " << name << std::endl;
			return false;
		}
		if (!_initShaderAttrs(it.second, gen, shader))
			return false;
		_shaders.insert(shader);
	}
	return true;
}

bool Pass::_initPost(const Config::node& conf, const genfunc& gen) {
	if (!Config::valid(conf))
		return true;

	for (const auto& it: conf) {
		const std::string& name = it.first.as<std::string>();
		const Post::ptr& post = PostMgr::inst().req(name);
		if (!post) {
			std::cout << "pass shader not found, " << name << std::endl;
			return false;
		}
		if (!_initShaderAttrs(it.second, gen, post->getMaterial()->getShader()))
			return false;
		_posts.insert(post);
	}
	return true;
}

bool Pass::_initState(const Config::node& conf) {
	if (Config::valid(conf)) {
		for (const auto& it: conf) {
			const std::string& key = it.first.as<std::string>();
			if (key == "clear")
				_stateClear(it.second);
			else if (key == "depth")
				_stateDepth(it.second);
		}
	}

	return true;
}

void Pass::_stateClear(const Config::node& conf) {
	const glm::vec3& bgcolor = EventMgr::inst().getBGColor();
	GLbitfield flags = 0;
	for (const auto& it: conf) {
		const std::string& arg = it.as<std::string>();
		if (arg == "color")
			flags |= GL_COLOR_BUFFER_BIT;
		else if (arg == "depth")
			flags |= GL_DEPTH_BUFFER_BIT;
		else if (arg == "stencil")
			flags |= GL_STENCIL_BUFFER_BIT;
	}

	_states.emplace_back([flags, bgcolor] {
		if (flags & GL_COLOR_BUFFER_BIT) {
			glClearColor(bgcolor.x, bgcolor.y, bgcolor.z, 1.0f);
		}
		glClear(flags);
	});
}

void Pass::_stateDepth(const Config::node& conf) {
	bool enable = conf[0].as<bool>();
	GLenum func = GL_LESS;
	if (enable) {
		const std::string& arg = conf[1].as<std::string>();
		if (arg == "less")
			func = GL_LESS;
		else if (arg == "lesseq")
			func = GL_LEQUAL;
	}

	_states.emplace_back([enable, func] {
		if (enable) {
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(func);
		}
		else
			glDisable(GL_DEPTH_TEST);
	});
}

void Pass::drawBegin() {
	if (_out)
		glBindFramebuffer(GL_FRAMEBUFFER, _out->getFBO());
	for (const auto& it: _states)
		it();
}

int Pass::drawPass(CommandQueue& cmds, const modelvec& models) {
	// TODO: 按照shader来收集cmd
	int total{0};
	int n;
	if (!_shaders.empty()) {
		for (auto& it: models) {
			n = it->draw(cmds, _name, _shaders);
			if (n < 0)
				return -1;
			total += n;
		}
	}

	for (const auto& it: _posts) {
		n = it->draw(cmds);
		if (n < 0)
			return -1;
		total += n;
	}

	// TODO: pass完成后需要设置回去？
	std::map<Frame::ptr, GLuint> texcache;
	for (auto& it: cmds) {
		const std::string& shader = it.material->getShader()->getName();
		const auto& find = _sattrs.find(shader);
		if (find == _sattrs.end())
			continue;

		const ShaderAttrs& attrs = find->second;
		for (const auto& itin: attrs.ins) {
			it.attrs.setAttr(itin.first, itin.second->getTexture());
		}
		for (const auto& itin: attrs.outs) {
			// TODO: output texture
		}
		for (const auto& itin: attrs.attrs) {
			it.attrs.updateAttrs(attrs.attrs);
		}
	}

	return total;
}

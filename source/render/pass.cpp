#include "pass.hpp"
#include "event.hpp"

Pass::ptr Pass::create(const Config::node& conf, framemap& frames) {
	Pass::ptr pass = std::shared_ptr<Pass>(new Pass());

	if (!pass->_initConf(conf))
		return {};
	if (!pass->_initFrame(conf, frames))
		return {};
	if (!pass->_initState(conf))
		return {};
	return pass;
}

bool Pass::_initConf(const Config::node& conf) {
	setName(conf["name"].as<std::string>());

	if (Config::valid(conf["camera"])) {
		_cam = conf["camera"].as<std::string>();
	}
	if (Config::valid(conf["shaders"])) {
		for (auto& it: conf["shaders"]) {
			Shader::ptr shader = ShaderMgr::inst().req(it.as<std::string>());
			if (!shader)
				return false;
			_shaders.insert(shader);
		}
	}

	if (Config::valid(conf["post"])) {
		_post = PostMgr::inst().req(conf["post"].as<std::string>());
		if (!_post)
			return false;
	}

	return true;
}

bool Pass::_initFrame(const Config::node& conf, framemap& frames) {
	if (Config::valid(conf["in"])) {
		for (auto& it: conf["in"]) {
			const auto& find = frames.find(it.as<std::string>());
			if (find == frames.end()) {
				std::cout << "pass input not found, " << it.as<std::string>();
				return false;
			}
			_ins.push_back(find->second);
		}
	}

	if (Config::valid(conf["out"])) {
		Frame::ptr frame;
		const std::string& out = conf["out"]["name"].as<std::string>();
		const auto& find = frames.find(out);
		if (find == frames.end()) {
			frame = Frame::create();
			for (const auto& it: conf["out"]["attach"]) {
				const std::string& attach = it.as<std::string>();
				if (attach == "texture")
					frame->attachTexture();
				else if (attach == "depst")
					frame->attachDepthStencil();
				else if (attach == "shadow")
					frame->attachShadowMap();
			}
			frames[out] = frame;
		}
		else {
			frame = find->second;
		}
		_out = frame;
	}

	return true;
}

bool Pass::_initState(const Config::node& conf) {
	if (Config::valid(conf["states"])) {
		for (const auto& it: conf["states"]) {
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
		_out->drawBegin();
	for (const auto& it: _states)
		it();
}
void Pass::drawEnd() {
	if (_out)
		_out->drawEnd();
}

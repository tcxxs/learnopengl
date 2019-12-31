#include "pass.hpp"
#include "system.hpp"

Pass::ptr Pass::create(const Config::node& conf) {
	Pass::ptr pass = std::make_shared<Pass>();

	// TODO: 可以支持generate和重复多次，针对bloom这种
	if (!pass->_initConf(conf))
		return {};
	if (!pass->_initRun(conf["run"]))
		return {};
	if (!pass->_initState(conf["states"]))
		return {};
	// TODO: 还可以增加一个cubes
	if (!pass->_initShader(conf["shaders"]))
		return {};
	if (!pass->_initProc(conf["procs"]))
		return {};
	if (!pass->_initOutput(conf["output"]))
		return {};
	return pass;
}

bool Pass::_initConf(const Config::node& conf) {
	setName(conf["name"].as<std::string>());
	if (Config::valid(conf["camera"])) {
		std::any ret = Config::guess(conf["camera"]);
		if (!ret.has_value()) {
			ERR("pass %s, camera %s, not valid\n", _name.c_str(), conf["camera"].Scalar().c_str());
			return false;
		}
		_cam = std::any_cast<Camera::ptr>(ret);
	}

	return true;
}

bool Pass::_initRun(const Config::node& conf) {
	if (!Config::valid(conf))
		return true;
	if (!conf.IsSequence() || conf.size() < 1) {
		ERR("pass %s, run arguments error", _name.c_str());
		return false;
	}

	const std::string& type = conf[0].as<std::string>();
	if (type == "once") {
		_run = [this]() -> bool {
			if (_run_once) {
				_run_once = false;
				return true;
			}
			else
				return false;
		};
	}
	else if (type == "if") {
		if (conf.size() < 2) {
			ERR("pass %s, run if arguments error", _name.c_str());
			return false;
		}

		bool ret = Config::guess(conf[1]).has_value();
		_run = [ret] {
			return ret;
		};
	}

	return true;
}

// TODO: 会不会可能相同shader，不同参数
bool Pass::_initShaderAttrs(const Config::node& conf, const Shader::ptr& shader) {
	if (!Config::valid(conf))
		return true;

	auto insert = _sattrs.try_emplace(shader->getName(), ShaderAttrs());
	if (!insert.second) {
		ERR("pass %s, shader %s, duplicate\n", _name.c_str(), shader->getName().c_str());
		return false;
	}
	ShaderAttrs& attrs = insert.first->second;

	GLint loc;
	for (const auto& it: conf) {
		const std::string& name = it.first.as<std::string>();
		loc = shader->getVar(name);
		if (loc < 0) {
			ERR("pass %s, shader %s, uniform %s, not found\n", _name.c_str(), shader->getName().c_str(), name.c_str());
			continue;
		}

		std::any val = Config::guess(it.second);
		if (!val.has_value()) {
			ERR("pass %s, shader %s, uniform %s, value %s, not valid\n", _name.c_str(), shader->getName().c_str(), name.c_str(), it.second.Scalar().c_str());
			return false;
		}
		if (val.type() == typeid(ShaderAttrs::frameattach)) {
			const auto& frame = std::any_cast<const ShaderAttrs::frameattach&>(val);
			attrs.frames[name] = frame;
		}
		else {
			attrs.attrs.setAttr(name, val);
		}
	}

	return true;
}

bool Pass::_initShader(const Config::node& conf) {
	if (!Config::valid(conf))
		return true;

	for (const auto& it: conf) {
		const std::string& name = it.first.as<std::string>();
		const Shader::ptr& shader = ShaderMgr::inst().req(name);
		if (!shader) {
			ERR("pass shader not found, %s", name.c_str());
			return false;
		}
		if (!_initShaderAttrs(it.second, shader))
			return false;
		_shaders.insert(shader);
	}
	return true;
}

bool Pass::_initProc(const Config::node& conf) {
	if (!Config::valid(conf))
		return true;

	for (const auto& it: conf) {
		const std::string& name = it.first.as<std::string>();
		const Process::ptr& proc = PostMgr::inst().req(name);
		if (!proc) {
			ERR("pass shader not found, %s", name.c_str());
			return false;
		}
		if (!_initShaderAttrs(it.second, proc->getMaterial()->getShader()))
			return false;
		_procs.insert(proc);
	}
	return true;
}

bool Pass::_initOutput(const Config::node& conf) {
	if (!Config::valid(conf))
		return true;

	std::any ret = Config::guess(conf["frame"]);
	if (!ret.has_value()) {
		ERR("pass %s, output %s, not valid\n", _name.c_str(), conf["frame"].Scalar().c_str());
		return false;
	}
	_outframe = std::any_cast<const Frame::ptr&>(ret);

	if (Config::valid(conf["colors"])) {
		for (const auto& it: conf["colors"]) {
			const std::string& fname = it.first.as<std::string>();
			const std::string& sname = it.second.as<std::string>();

			const Frame::Attachment& attach = _outframe->getAttach(fname);
			if (!attach.type) {
				ERR("pass %s, output %s, attach %s not found\n", _name.c_str(), _outframe->getName().c_str(), fname.c_str());
				return false;
			}
			_outcolors[attach.index] = sname;

			//for (const auto& it: _shaders) {
			//	if (floc != it->getVar(sname)) {
			//		std::printf("pass %s, output %s, shader %s, %s(%d) -> %s(%d) color index error\n", _name.c_str(), it->getName().c_str(),
			//			conf["frame"].Scalar().c_str(), sname.c_str(), it->getVar(sname), fname.c_str(), floc);
			//		return false;
			//	}
			//}
			//for (const auto& it: _posts) {
			//	const Shader::ptr& shader = it->getMaterial()->getShader();
			//	if (floc != shader->getVar(sname)) {
			//		std::printf("pass %s, output %s, post %s, %s(%d) -> %s(%d) color index error\n", _name.c_str(), it->getName().c_str(),
			//			conf["frame"].Scalar().c_str(), sname.c_str(), shader->getVar(sname), fname.c_str(), floc);
			//		return false;
			//	}
			//}
		}
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
			else if (key == "cull")
				_stateCull(it.second);
		}
	}

	return true;
}

void Pass::_stateClear(const Config::node& conf) {
	GLbitfield flags{0};
	glm::vec3 color{0.0f};
	float depth{1.0f};
	for (const auto& it: conf) {
		if (it.IsSequence()) {
			const std::string& arg = it[0].as<std::string>();
			if (arg == "color") {
				flags |= GL_COLOR_BUFFER_BIT;
				color = it[1].as<glm::vec3>();
			}
			else if (arg == "depth") {
				flags |= GL_DEPTH_BUFFER_BIT;
				depth = it[1].as<float>();
			}
			else if (arg == "stencil")
				flags |= GL_STENCIL_BUFFER_BIT;
		}
		else {
			const std::string& arg = it.as<std::string>();
			if (arg == "color") {
				flags |= GL_COLOR_BUFFER_BIT;
				color = SystemMgr::inst().getBGColor();
			}
			else if (arg == "depth")
				flags |= GL_DEPTH_BUFFER_BIT;
			else if (arg == "stencil")
				flags |= GL_STENCIL_BUFFER_BIT;
		}
	}

	_states.emplace_back([flags, color, depth] {
		if (flags & GL_COLOR_BUFFER_BIT) {
			glClearColor(color.x, color.y, color.z, 1.0f);
		}
		if (flags & GL_DEPTH_BUFFER_BIT) {
			glClearDepth(depth);
		}
		glClear(flags);
	});
}

void Pass::_stateDepth(const Config::node& conf) {
	const std::string& mode = conf[0].as<std::string>();
	bool enable = false;
	bool write = false;
	if (mode == "update") {
		enable = true;
		write = true;
	}
	else if (mode == "test") {
		enable = true;
	}

	GLenum func = GL_LESS;
	if (enable) {
		if (conf.size() > 1) {
			const std::string& arg = conf[1].as<std::string>();
			if (arg == "less")
				func = GL_LESS;
			else if (arg == "lesseq")
				func = GL_LEQUAL;
		}
		else {
			func = GL_LESS;
		}
	}

	_states.emplace_back([enable, write, func] {
		if (enable) {
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(func);
			glDepthMask(write);
		}
		else
			glDisable(GL_DEPTH_TEST);
	});
}

void Pass::_stateCull(const Config::node& conf) {
	const std::string& name = conf.as<std::string>();
	GLenum face = GL_BACK;
	if (name == "front")
		face = GL_FRONT;

	_states.emplace_back([face] {
		glEnable(GL_CULL_FACE);
		glCullFace(face);
	});
}

void Pass::drawBegin() {
	if (_outframe) {
		glBindFramebuffer(GL_FRAMEBUFFER, _outframe->getFBO());
		if (_outcolors.empty()) {
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
		}
		else {
			// cmd decide draw buffers
		}
	}
	else {
		glDrawBuffer(GL_BACK);
	}

	std::pair<int, int> view = getView();
	glViewport(0, 0, GLsizei(view.first), GLsizei(view.second));
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	for (const auto& it: _states)
		it();
}

void Pass::drawEnd() {
	// TODO: 可以只dirty用到的color
	if (_outframe) {
		_outframe->setDirtyAll();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

	for (const auto& it: _procs) {
		n = it->draw(cmds);
		if (n < 0)
			return -1;
		total += n;
	}

	// TODO: pass完成后需要设置回去？
	std::map<Frame::ptr, GLuint> texcache;
	for (auto& it: cmds) {
		const Shader::ptr& shader = it.material->getShader();

		const auto& find = _sattrs.find(shader->getName());
		if (find != _sattrs.end()) {
			const ShaderAttrs& attrs = find->second;
			for (const auto& itf: attrs.frames) {
				it.attrs.setAttr(itf.first, itf.second.first->getTexture(itf.second.second));
			}
			for (const auto& itin: attrs.attrs) {
				it.attrs.updateAttrs(attrs.attrs);
			}
		}

		if (_outframe) {
			if (_outcolors.empty()) {
				// draw color0
			}
			else {
				std::map<GLuint, GLenum> outs;
				GLint maxloc{0};
				for (const auto& it: _outcolors) {
					GLint sloc = shader->getVar(it.second);
					if (sloc >= 0) {
						outs[sloc] = GL_COLOR_ATTACHMENT0 + it.first;
						if (sloc > maxloc)
							maxloc = sloc;
					}
				}

				if (outs.empty()) {
					// 无对应输出
					it.buffs.push_back(GL_NONE);
				}
				else {
					// 构造buffers列表
					for (int i = 0; i < (int)maxloc + 1; ++i) {
						const auto& find = outs.find(i);
						if (find == outs.end())
							it.buffs.push_back(GL_NONE);
						else
							it.buffs.push_back(find->second);
					}
				}
			}
		}
		else {
			// draw backbuffer
		}
	}

	return total;
}

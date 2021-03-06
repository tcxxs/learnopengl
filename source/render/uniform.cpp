#include "uniform.hpp"

UniformProto::ptr UniformProto::create(const std::string& name, const GLint size, const UniformProto::glinfo& vars) {
	if (_confs.root().IsNull()) {
		std::filesystem::path path = std::filesystem::current_path() / "resource" / "uniforms.yml";
		if (!_confs.load(path)) {
			ERR("uniforms config error");
			return {};
		}
	}
	Config::node conf = _confs[name];
	if (!Config::valid(conf)) {
		ERR("uniforms config not find, %s", name.c_str());
		return {};
	}

	UniformProto::ptr proto = std::make_shared<UniformProto>();
	proto->setName(name);

	proto->_size = size;
	if (conf.size() != vars.size()) {
		ERR("uniform variables number diffrent: %s", name.c_str());
		return {};
	}
	for (const auto& it: conf) {
		const std::string& var = it.first.as<std::string>();
		const auto& gl = vars.find(var);
		if (gl == vars.end()) {
			ERR("uniform variable not define: %s", var.c_str());
			return {};
		}
		const typeinfo& info = _types[it.second.as<std::string>()];
		proto->_vars.emplace(var, std::make_tuple(info, gl->second));
	}

	return proto;
}

UniformInst::ptr UniformInst::create(const proto_ptr& proto) {
	UniformInst::ptr uniform = std::make_shared<UniformInst>();

	glGenBuffers(1, &uniform->_ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, uniform->_ubo);
	glBufferData(GL_UNIFORM_BUFFER, proto->getSize(), nullptr, GL_DYNAMIC_READ);
	void* buff = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	memset(buff, 0, proto->getSize());
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	return uniform;
}

UniformInst::~UniformInst() {
	if (_ubo) {
		glDeleteBuffers(1, &_ubo);
		_ubo = 0;
	}
}

void UniformInst::bind(const GLuint point) {
	glBindBuffer(GL_UNIFORM_BUFFER, _ubo);
	glBindBuffersBase(GL_UNIFORM_BUFFER, point, 1, &_ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

bool UniformInst::setVar(int offset, const void* addr, const int size) {
	glBindBuffer(GL_UNIFORM_BUFFER, _ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, size, addr);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	return true;
}

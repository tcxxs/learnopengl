#include "shader.hpp"
#include "render/uniform.hpp"

Shader::ptr Shader::create(const std::string& name) {
	Shader::ptr shader = std::make_shared<Shader>();
	shader->setName(name);

	if (!shader->_loadProgram())
		return {};
	if (!shader->_loadVertex())
		return {};
	if (!shader->_loadUniform())
		return {};
	if (!shader->_loadFragdata())
		return {};

	return shader;
}

Shader::~Shader() {
	if (_prog) {
		glDeleteProgram(_prog);
		_prog = 0;
	}
}

bool Shader::_loadShader(const std::string& ext, int type, GLuint& shader) {
	std::filesystem::path path = std::filesystem::current_path() / "resource" / "shader" / (_name + ext);
	if (!std::filesystem::exists(path)) {
		shader = 0;
		return true;
	}

	std::string content;
	if (!readFile(path, content)) {
		return false;
	}
	const char* cstr = content.c_str();

	shader = glCreateShader(type);
	glShaderSource(shader, 1, &cstr, nullptr);
	glCompileShader(shader);

	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (!success) {
		GLchar infoLog[512];
		glGetShaderInfoLog(shader, 512, nullptr, infoLog);
		ERR("load shader: %s, compile fail: %s", _name.c_str(), infoLog);
		return false;
	}

	return true;
}

bool Shader::_loadProgram() {
	GLuint vs{0}, gs{0}, fs{0};
	if (!_loadShader(".vert", GL_VERTEX_SHADER, vs)) {
		return false;
	}
	if (!_loadShader(".geom", GL_GEOMETRY_SHADER, gs)) {
		return false;
	}
	if (!_loadShader(".frag", GL_FRAGMENT_SHADER, fs)) {
		return false;
	}

	_prog = glCreateProgram();
	glAttachShader(_prog, vs);
	if (gs)
		glAttachShader(_prog, gs);
	glAttachShader(_prog, fs);
	glLinkProgram(_prog);

	GLint success;
	glGetProgramiv(_prog, GL_LINK_STATUS, &success);
	if (!success) {
		GLchar infoLog[512];
		glGetProgramInfoLog(_prog, 512, nullptr, infoLog);
		ERR("load program, link fail: %s", infoLog);
		return false;
	}

	glDeleteShader(vs);
	if (gs)
		glDeleteShader(gs);
	glDeleteShader(fs);

	return true;
}

bool Shader::_loadVertex() {
	const auto& vertall = VertexProtoMgr::inst().container();
	VertexInst::ptr vertfind;

	GLint resources;
	std::string name(50, 0);
	const GLenum props[] = {GL_LOCATION, GL_TYPE};
	GLint values[2];
	glGetProgramInterfaceiv(_prog, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &resources);
	for (int i = 0; i < resources; ++i) {
		glGetProgramResourceiv(_prog, GL_PROGRAM_INPUT, i, 2, props, 2, nullptr, values);
		if (values[0] < 0)
			continue;

		glGetProgramResourceName(_prog, GL_PROGRAM_INPUT, i, (GLsizei)name.capacity(), nullptr, name.data());

		vertfind = nullptr;
		for (const auto& it: _verts) {
			const VertexProto::attrinfo& type = it.second->prototype()->findInfo(name.c_str());
			if (std::get<0>(std::get<0>(type)) > 0) {
				vertfind = it.second;
				break;
			}
		}
		if (!vertfind) {
			for (const auto& it: vertall) {
				const VertexProto::attrinfo& type = it.second->findInfo(name.c_str());
				if (std::get<0>(std::get<0>(type)) > 0) {
					vertfind = it.second->instance();
					_verts[it.second->getName()] = vertfind;
					break;
				}
			}
		}
		if (!vertfind) {
			ERR("shader vertex attribute not define: %s", name.c_str());
			return false;
		}
		vertfind->setAttr(name.c_str(), values[0], values[1]);
	}

	return true;
}

bool Shader::_loadUniform() {
	GLint resources;
	std::string name(50, '\0');
	std::string uname, vname;
	size_t pos;

	const GLenum props_uniform[] = {GL_TYPE, GL_LOCATION};
	GLint values_uniform[2];
	GLint tex{0};
	GLenum type{0};
	glGetProgramInterfaceiv(_prog, GL_UNIFORM, GL_ACTIVE_RESOURCES, &resources);
	for (int i = 0; i < resources; ++i) {
		glGetProgramResourceiv(_prog, GL_UNIFORM, i, 2, props_uniform, 2, nullptr, values_uniform);
		if (values_uniform[1] < 0)
			continue;

		glGetProgramResourceName(_prog, GL_UNIFORM, i, (GLsizei)name.capacity(), nullptr, name.data());
		pos = name.find('[');
		if (pos == std::string::npos)
			uname = name.c_str();
		else
			uname = name.substr(0, pos);

		type = 0;
		switch (values_uniform[0]) {
		case GL_SAMPLER_2D: type = GL_TEXTURE_2D; break;
		case GL_SAMPLER_CUBE: type = GL_TEXTURE_CUBE_MAP; break;
		}

		if (type) {
			_samplers[values_uniform[1]] = std::make_pair(tex, type);
			tex += 1;
		}
		_vars[uname.c_str()] = values_uniform[1];
	}

	const GLenum props_block[]{GL_NUM_ACTIVE_VARIABLES}, props_active[]{GL_BUFFER_DATA_SIZE, GL_ACTIVE_VARIABLES}, props_var[]{GL_OFFSET};
	GLint values_block[1], values_var[1];
	GLuint bind{0};
	glGetProgramInterfaceiv(_prog, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &resources);
	for (int i = 0; i < resources; ++i) {
		glGetProgramResourceiv(_prog, GL_UNIFORM_BLOCK, i, 1, props_block, 1, nullptr, values_block);
		glGetProgramResourceName(_prog, GL_UNIFORM_BLOCK, i, (GLsizei)name.capacity(), nullptr, name.data());
		glUniformBlockBinding(_prog, i, bind);
		_vars.emplace(name.c_str(), bind);
		++bind;

		pos = name.find('[');
		if (pos == std::string::npos)
			uname = name.c_str();
		else
			uname = name.substr(0, pos);
		if (UniformProtoMgr::inst().get(uname))
			continue;

		std::vector<GLint> values_active(values_block[0] + 1);
		std::map<std::string, GLint> vars;
		glGetProgramResourceiv(_prog, GL_UNIFORM_BLOCK, i, 2, props_active, values_block[0] + 1, nullptr, values_active.data());
		for (int j = 0; j < values_block[0]; ++j) {
			glGetProgramResourceiv(_prog, GL_UNIFORM, values_active[j + 1], 1, props_var, 1, nullptr, values_var);
			glGetProgramResourceName(_prog, GL_UNIFORM, values_active[j + 1], (GLsizei)name.capacity(), nullptr, name.data());
			pos = name.find(uname + '.');
			if (pos == 0)
				vname = name.substr(uname.size() + 1).c_str();
			else
				vname = name.c_str();
			vars.emplace(vname, values_var[0]);
		}
		UniformProtoMgr::inst().req(uname, values_active[0], vars);
	}

	return true;
}

bool Shader::_loadFragdata()
{
	GLint resources;
	std::string name(50, '\0');

	const GLenum props[] = {GL_LOCATION};
	GLint values[1];
	glGetProgramInterfaceiv(_prog, GL_PROGRAM_OUTPUT, GL_ACTIVE_RESOURCES, &resources);
	for (int i = 0; i < resources; ++i) {
		glGetProgramResourceiv(_prog, GL_PROGRAM_OUTPUT, i, 1, props, 1, nullptr, values);
		if (values[0] < 0)
			continue;

		glGetProgramResourceName(_prog, GL_PROGRAM_OUTPUT, i, (GLsizei)name.capacity(), nullptr, name.data());
		_vars[name.c_str()] = values[0];
	}

	return true;
}

void Shader::use() {
	glUseProgram(_prog);
	for (const auto& it: _samplers) {
		glBindTextureUnit(it.second.first, Texture::getDefault(it.second.second));
		glUniform1i(it.first, it.second.first);
	}
}

void Shader::setVar(const GLuint& loc, const std::any& var) {
	if (var.type() == typeid(int)) {
		setVar(loc, std::any_cast<const int&>(var));
	}
	else if (var.type() == typeid(float)) {
		setVar(loc, std::any_cast<const float&>(var));
	}
	else if (var.type() == typeid(std::string)) {
		setVar(loc, std::any_cast<const std::string&>(var));
	}
	else if (var.type() == typeid(glm::vec3)) {
		setVar(loc, std::any_cast<const glm::vec3&>(var));
	}
	else if (var.type() == typeid(std::vector<glm::vec3>)) {
		setVar(loc, std::any_cast<const std::vector<glm::vec3>&>(var));
	}
	else if (var.type() == typeid(glm::mat4)) {
		setVar(loc, std::any_cast<const glm::mat4&>(var));
	}
	else if (var.type() == typeid(std::vector<glm::mat4>)) {
		setVar(loc, std::any_cast<const std::vector<glm::mat4>&>(var));
	}
	else if (var.type() == typeid(Texture::val)) {
		setVar(loc, std::any_cast<const Texture::val&>(var));
	}
	else {
		ERR("shader var unknow, loc: %d, type: %s", loc, var.type().name());
	}
}

bool Shader::bindVertex(const std::string& name, const GLuint vao, const GLuint vbo) {
	const auto& it = _verts.find(name);
	if (it == _verts.end()) {
		ERR("shader, bind vertex, not found, %s", name.c_str());
		return false;
	}

	return it->second->bindBuffer(vao, vbo);
}

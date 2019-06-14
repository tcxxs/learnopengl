#include "shader.hpp"

Shader::ptr Shader::create(const std::string& name) {
	Shader::ptr shader = std::shared_ptr<Shader>(new Shader());
	shader->setName(name);

	if (!shader->_loadProgram())
		return {};

	if (oglError())
		return {};

	return shader;
}

Shader::~Shader() {
	if (_prog) {
		glDeleteProgram(_prog);
		_prog = 0;
	}
}

bool Shader::_loadShader(int type, GLuint& shader) {
	std::filesystem::path path = std::filesystem::current_path() / "resource" / "shader";
	switch (type) {
	case GL_VERTEX_SHADER:
		path /= _name + ".vs";
		break;
	case GL_FRAGMENT_SHADER:
		path /= _name + ".fs";
		break;
	default:
		std::cout << "load shader, unknow type" << type << std::endl;
		return false;
	}

	std::string content;
	if (!readFile(path, content)) {
		return false;
	}
	const char* cstr = content.c_str();

	shader = glCreateShader(type);
	glShaderSource(shader, 1, &cstr, NULL);
	glCompileShader(shader);

	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (!success) {
		GLchar infoLog[512];
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cout << "load shader: " << _name
		          << ", compile fail: \n"
		          << infoLog << std::endl;
		return false;
	}

	return true;
}

bool Shader::_loadProgram() {
	GLuint vs, fs;
	if (!_loadShader(GL_VERTEX_SHADER, vs)) {
		return false;
	}
	if (!_loadShader(GL_FRAGMENT_SHADER, fs)) {
		return false;
	}

	_prog = glCreateProgram();
	glAttachShader(_prog, vs);
	glAttachShader(_prog, fs);
	glBindAttribLocation(_prog, POS_LOC, POS_NAME);
	glBindAttribLocation(_prog, UV_LOC, UV_NAME);
	glBindAttribLocation(_prog, NORMAL_LOC, NORMAL_NAME);
	glLinkProgram(_prog);

	GLint success;
	glGetProgramiv(_prog, GL_LINK_STATUS, &success);
	if (!success) {
		GLchar infoLog[512];
		glGetProgramInfoLog(_prog, 512, NULL, infoLog);
		std::cout << "load program, link fail: \n"
		          << infoLog << std::endl;
		return false;
	}

	glDeleteShader(vs);
	glDeleteShader(fs);

	std::string name;
	GLenum type{0};
	GLint count{0}, size{0};
	GLsizei namesize{20}, length{0};
	glGetProgramiv(_prog, GL_ACTIVE_ATTRIBUTES, &count);
	for (auto i = 0; i < count; i++) {
		name.clear();
		name.resize(namesize, 0);
		glGetActiveAttrib(_prog, (GLuint)i, namesize, &length, &size, &type, name.data());
		_vars[name.c_str()] = glGetAttribLocation(_prog, name.data());
	}

	glGetProgramiv(_prog, GL_ACTIVE_UNIFORMS, &count);
	for (auto i = 0; i < count; i++) {
		name.clear();
		name.resize(namesize, 0);
		glGetActiveUniform(_prog, (GLuint)i, namesize, &length, &size, &type, name.data());
		_vars[name.c_str()] = i;
	}

	GLint loc;
	loc = getVar(POS_NAME);
	if (loc >= 0 && loc != POS_LOC) {
		std::cout << "load program, pos location error" << std::endl;
		return false;
	}
	loc = getVar(UV_NAME);
	if (loc >= 0 && loc != UV_LOC) {
		std::cout << "load program, uv location error" << std::endl;
		return false;
	}
	loc = getVar(NORMAL_NAME);
	if (loc >= 0 && loc != NORMAL_LOC) {
		std::cout << "load program, normal location error" << std::endl;
		return false;
	}

	return true;
}

void Shader::use() {
	glUseProgram(_prog);
	_tex = 0;
}

void Shader::setVar(const GLuint& loc, const std::any& var) {
	if (var.type() == typeid(int)) {
		setVar(loc, std::any_cast<const int&>(var));
	}
	else if (var.type() == typeid(float)) {
		setVar(loc, std::any_cast<const float&>(var));
	}
	else if (var.type() == typeid(glm::vec3)) {
		setVar(loc, std::any_cast<const glm::vec3&>(var));
	}
	else if (var.type() == typeid(Texture::ptr)) {
		setVar(loc, std::any_cast<const Texture::ptr&>(var));
	}
	else {
		std::cout << "shader var unknow, loc: " << loc << ", type: " << var.type().name() << std::endl;
	}
}
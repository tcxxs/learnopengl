#include "shader.hpp"
#include "render/texture.hpp"

Shader::ptr Shader::create(const std::string& name) {
	Shader::ptr shader = std::shared_ptr<Shader>(new Shader());
	shader->setName(name);

	if (!shader->loadProgram())
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
	std::filesystem::path path = std::filesystem::current_path()/"resource"/"shader";
	switch(type) {
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

	if (!success)
	{
		GLchar infoLog[512];
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cout << "load shader, compile fail: \n" << infoLog << std::endl;
		return false;
	}

	return true;
}

bool Shader::loadProgram() {
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
	glLinkProgram(_prog);

	GLint success;
	glGetProgramiv(_prog, GL_LINK_STATUS, &success);
	if (!success)
	{
		GLchar infoLog[512];
		glGetProgramInfoLog(_prog, 512, NULL, infoLog);
		std::cout << "load program, link fail: \n" << infoLog << std::endl;
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
		_vars[name.c_str()] = i;
	}

	glGetProgramiv(_prog, GL_ACTIVE_UNIFORMS, &count);
	for (auto i = 0; i < count; i++) {
		name.clear();
		name.resize(namesize, 0);
		glGetActiveUniform(_prog, (GLuint)i, namesize, &length, &size, &type, name.data());
		_vars[name.c_str()] = i;
	}

	return true;
}

bool Shader::useProgram() {
	glUseProgram(_prog);
	return true;
}

void Shader::setVars(const Attributes& attrs) {
	GLint loc{0};
	GLuint tex{0};
	for (const auto& it: attrs) {
		loc = getVar(it.first);
		if (loc < 0)
			continue;

		if (it.second.type() == typeid(float)) {
			glUniform1f(loc, std::any_cast<const float&>(it.second));
		}
		else if (it.second.type() == typeid(glm::vec3)) {
			glUniform3fv(loc, 1, glm::value_ptr(std::any_cast<const glm::vec3&>(it.second)));
		}
		else if (it.second.type() == typeid(Texture::ptr)) {
			glActiveTexture(GL_TEXTURE0 + tex);
			glBindTexture(GL_TEXTURE_2D, std::any_cast<const Texture::ptr&>(it.second)->getTexture());
			glUniform1i(loc, tex);
			tex += 1;
		}
		else {
			std::cout << "shader var unknow, name: " << it.first << ", type: " << it.second.type().name() << std::endl;
		}
	}
}
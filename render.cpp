#include "render.hpp"

Shader::~Shader() {
	glDeleteProgram(_prog);
}

bool Shader::_loadShader(int type, GLuint& shader) {
	std::filesystem::path path = std::filesystem::current_path() / "shader";
	switch(type) {
		case GL_VERTEX_SHADER:
			path /= name + ".vs";
			break;
		case GL_FRAGMENT_SHADER:
			path /= name + ".fs";
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
	return true;
}

bool Shader::useProgram() {
	glUseProgram(_prog);

	GLint success;
	glGetProgramiv(_prog, GL_LINK_STATUS, &success);

	if (!success)
	{
		GLchar infoLog[512];
		glGetProgramInfoLog(_prog, 512, NULL, infoLog);
		std::cout << "use program, fail: \n" << infoLog << std::endl;
		return false;
	}

	return true;
}

void onRender()
{
	glClearColor(BG_COLOR);
	glClear(GL_COLOR_BUFFER_BIT);

	GLfloat vertices[] = {
	-0.5f, -0.5f, 0.0f,
	 0.5f, -0.5f, 0.0f,
	 0.0f,  0.5f, 0.0f
	};

	static GLuint VBO = 0;
	if (!VBO) {
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	}

	static GLuint VAO = 0;
	if (!VAO) {
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
		glEnableVertexAttribArray(0);
	}

	Shader shader("simple");
	if (!shader.loadProgram()) {
		return;
	}
	if (!shader.useProgram()) {
		return;
	}
	
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

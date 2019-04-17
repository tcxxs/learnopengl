#pragma once

#include <string>
#include <filesystem>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "config.hpp"
#include "resource.hpp"

class Shader {
public:
	Shader(const std::string& shader) : name(shader), _prog(0) {}
	~Shader();

	bool loadProgram();
	bool useProgram();
private:
	bool _loadShader(int type, GLuint& shader);

public:
	std::string name;
private:
	GLuint _prog;
};

void onRender();
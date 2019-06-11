#pragma once

#include "glad/glad.h"
#include "config.hpp"
#include "render/material.hpp"
#include "utils/utils.hpp"

struct Command {
	GLuint vao;
	GLsizei ibosize;
	glm::mat4 model;
	Material::ptr material;
	Attributes attrs;
};

using CommandQueue = std::vector<Command>;
#pragma once

#include "glad/glad.h"
#include "config.hpp"
#include "render/material.hpp"
#include "utils/utils.hpp"

struct Command {
	GLuint vao{0};
	GLsizei ibosize{0};
	glm::mat4 model;
	Material::ptr material;
	Attributes attrs;
};

using CommandQueue = std::vector<Command>;
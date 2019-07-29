#pragma once

#include "glad/glad.h"
#include "config.hpp"
#include "render/material.hpp"
#include "utils/utils.hpp"

struct Command {
	GLuint vao{0};
	GLsizei inds{0};
	GLsizei verts{0};
	int ins{0};
	glm::mat4 model;
	Material::ptr material;
	Attributes attrs;
	std::vector<GLenum> buffs;
};

using CommandQueue = std::vector<Command>;
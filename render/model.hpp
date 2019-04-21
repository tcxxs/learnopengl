#pragma once

#include "glad/glad.h"
#include "event.hpp"
#include "utils/resource.hpp"

class Mesh: public Res<Mesh> {
public:
	static ptr create(const std::vector<GLfloat>& verts, const std::vector<GLuint>& inds);
	virtual ~Mesh();

	void draw();

private:
	GLuint _vbo{0};
	GLuint _ibo{0};
	GLuint _vao{0};
};

using MeshMgr = ResMgr<std::string, Mesh>;
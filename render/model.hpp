#pragma once

#include "glad/glad.h"
#include "event.hpp"
#include "utils/resource.hpp"

class Mesh: public Res<Mesh> {
public:
	static ptr create(GLfloat verts[], int len);
	virtual ~Mesh();

	void draw();

protected:
	Mesh() : _vbo(0), _vao(0) {}

private:
	GLuint _vbo;
	GLuint _vao;
};

using MeshMgr = ResMgr<std::string, Mesh>;
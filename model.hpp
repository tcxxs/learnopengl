#pragma once

#include "glad/glad.h"

class Mesh {
public:
	Mesh() = delete;
	Mesh(GLfloat verts[], int len);
	~Mesh();

	void draw();
private:
	GLuint _vbo;
	GLuint _vao;
};

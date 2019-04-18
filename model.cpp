#include "model.hpp"

Mesh::Mesh(GLfloat verts[], int len) : _vbo(0), _vao(0) {
	glGenBuffers(1, &_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, len, verts, GL_STATIC_DRAW);

	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

Mesh::~Mesh() {
	if (_vbo)
		glDeleteBuffers(1, &_vbo);
	if (_vao)
		glDeleteVertexArrays(1, &_vao);
}

void Mesh::draw() {
	glBindVertexArray(_vao);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}
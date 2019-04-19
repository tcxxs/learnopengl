#include "model.hpp"

Mesh::ptr Mesh::create(GLfloat verts[], int len) {
	Mesh::ptr mesh = std::shared_ptr<Mesh>(new Mesh());

	glGenBuffers(1, &mesh->_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->_vbo);
	glBufferData(GL_ARRAY_BUFFER, len, verts, GL_STATIC_DRAW);

	glGenVertexArrays(1, &mesh->_vao);
	glBindVertexArray(mesh->_vao);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	if (oglError())
		return nullptr;

	return mesh;
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
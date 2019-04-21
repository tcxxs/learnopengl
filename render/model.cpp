#include "model.hpp"

Mesh::ptr Mesh::create(const std::vector<GLfloat>& verts, const std::vector<GLuint>& inds) {
	Mesh::ptr mesh = std::shared_ptr<Mesh>(new Mesh());

	glGenVertexArrays(1, &mesh->_vao);
	glGenBuffers(1, &mesh->_vbo);
	glGenBuffers(1, &mesh->_ibo);

	glBindVertexArray(mesh->_vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->_vbo);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(GLfloat), verts.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size() * sizeof(GLuint), inds.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	if (oglError())
		return {};

	return std::move(mesh);
}

Mesh::~Mesh() {
	if (_vbo)
		glDeleteBuffers(1, &_vbo);
	if (_vao)
		glDeleteVertexArrays(1, &_vao);
}

void Mesh::draw() {
	glBindVertexArray(_vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
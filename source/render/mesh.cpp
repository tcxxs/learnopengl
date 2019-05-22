#include "model.hpp"

Mesh::ptr Mesh::create(const std::string& name, const std::vector<GLfloat>& verts, const std::vector<GLuint>& inds) {
	Mesh::ptr mesh = std::shared_ptr<Mesh>(new Mesh());
	mesh->setName(name);

	glGenVertexArrays(1, &mesh->_vao);
	glGenBuffers(1, &mesh->_vbo);
	glGenBuffers(1, &mesh->_ibo);

	glBindVertexArray(mesh->_vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->_vbo);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(GLfloat), verts.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size() * sizeof(GLuint), inds.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(POS_LOC, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)0);
	glVertexAttribPointer(UV_LOC, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
	glVertexAttribPointer(NORMAL_LOC, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(8 * sizeof(GLfloat)));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	if (oglError())
		return {};

	return mesh;
}

Mesh::~Mesh() {
	if (_vao) {
		glDeleteVertexArrays(1, &_vao);
		_vao = 0;
	}
	if (_vbo) {
		glDeleteBuffers(1, &_vbo);
		_vbo = 0;
	}
	if (_ibo) {
		glDeleteBuffers(1, &_ibo);
		_ibo = 0;
	}
}
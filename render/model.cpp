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

Model::ptr Model::create(const Mesh::ptr& _mesh, const Shader::ptr& _shader) {
	Model::ptr model = Model::ptr(new Model());

	model->_mesh = _mesh;
	model->_shader = _shader;

	return std::move(model);
}

Model::~Model() {
	_mesh = nullptr;
	_shader = nullptr;
}

void Model::draw() {
	_shader->useProgram();
	glUniform4f(_shader->getVar("color"), 0.0f, 0.5f, 0.0f, 1.0f);

	glBindVertexArray(_mesh->getVAO());
	glEnableVertexAttribArray(_shader->getVar("pos"));
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

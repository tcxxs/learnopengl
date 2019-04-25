#include "model.hpp"

Mesh::ptr Mesh::create(const std::vector<GLfloat>& verts, const std::vector<GLuint>& inds) {
	Mesh::ptr mesh = std::shared_ptr<Mesh>(new Mesh());

	glGenBuffers(1, &mesh->_vbo);
	glGenBuffers(1, &mesh->_ibo);

	glBindBuffer(GL_ARRAY_BUFFER, mesh->_vbo);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(GLfloat), verts.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size() * sizeof(GLuint), inds.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	if (oglError())
		return {};

	return std::move(mesh);
}

Mesh::~Mesh() {
	if (_vbo) {
		glDeleteBuffers(1, &_vbo);
		_vbo = 0;
	}
}

Model::ptr Model::create(const Mesh::ptr& mesh, const Shader::ptr& shader, const Texture::ptr& tex) {
	Model::ptr model = Model::ptr(new Model());

	model->_mesh = mesh;
	model->_shader = shader;
	model->_tex = tex;
	model->_pos = shader->getVar("pos");
	if (model->_pos < 0) {
		std::cout << "shader bind, no pos" << std::endl;
		return {};
	}
	model->_color = shader->getVar("color");
	if (model->_color < 0) {
		std::cout << "shader bind, no color" << std::endl;
		return {};
	}
	model->_uv = shader->getVar("uv");
	if (model->_uv < 0) {
		std::cout << "shader bind, no uv" << std::endl;
		return {};
	}

	glGenVertexArrays(1, &model->_vao);
	glBindVertexArray(model->_vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->getVBO());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->getIBO());
	glVertexAttribPointer(model->_pos, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
	glVertexAttribPointer(model->_color, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	glVertexAttribPointer(model->_uv, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return std::move(model);
}

Model::~Model() {
	_mesh = nullptr;
	_shader = nullptr;
	_tex = nullptr;

	if (_vao) {
		glDeleteVertexArrays(1, &_vao);
		_vao = 0;
	}
}

void Model::draw() {
	_shader->useProgram();
	//glUniform4f(_shader->getVar("color"), 0.0f, 0.5f, 0.0f, 1.0f);

	glBindTexture(GL_TEXTURE_2D, _tex->getTexture());
	glBindVertexArray(_vao);
	glEnableVertexAttribArray(_pos);
	glEnableVertexAttribArray(_color);
	glEnableVertexAttribArray(_uv);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

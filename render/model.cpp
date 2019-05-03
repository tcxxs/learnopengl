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
	model->_lpos = shader->getVar("pos");
	model->_lcolor = shader->getVar("color");
	model->_luv = shader->getVar("uv");
	model->_ltex = shader->getVar("tex");

	glGenVertexArrays(1, &model->_vao);
	glBindVertexArray(model->_vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->getVBO());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->getIBO());
	if (model->_lpos >= 0)
		glVertexAttribPointer(model->_lpos, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
	if (model->_lcolor >= 0)
		glVertexAttribPointer(model->_lcolor, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	if (model->_luv >= 0)
		glVertexAttribPointer(model->_luv, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

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

void Model::draw(const glm::mat4& view, const glm::mat4& proj) {
	_shader->useProgram();

	glUniformMatrix4fv(_shader->getVar("model"), 1, GL_FALSE, glm::value_ptr(_mat));
	glUniformMatrix4fv(_shader->getVar("view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(_shader->getVar("proj"), 1, GL_FALSE, glm::value_ptr(proj));

	glBindVertexArray(_vao);
	if (_lpos >= 0)
		glEnableVertexAttribArray(_lpos);
	if (_lcolor >= 0)
		glEnableVertexAttribArray(_lcolor);
	if (_tex && _luv >= 0 && _ltex >= 0) {
		glEnableVertexAttribArray(_luv);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _tex->getTexture());
		glUniform1i(_ltex, 0);
	}

	GLint loc{0};
	for (auto it : _vars) {
		loc = _shader->getVar(it.first);
		if (loc < 0)
			continue;

		if (it.second.type() == typeid(glm::vec3)) {
			glUniform3fv(loc, 1, glm::value_ptr(std::any_cast<glm::vec3>(it.second)));
		}
		else {
			std::cout << "model var unknow, name: " << it.first << ", type: " << it.second.type().name() << std::endl;
		}
	}

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

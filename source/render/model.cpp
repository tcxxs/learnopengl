#include "model.hpp"

Model::ptr Model::create(const std::string& name) {
	Model::ptr model = Model::ptr(new Model());
	std::filesystem::path path = std::filesystem::current_path() / "resource" / "model" / (name + ".yml");
	if (!model->_conf.load(path)) {
		std::cout << "model config error, " << path << std::endl;
		return {};
	}

	std::string key;
	key = model->_conf["mesh"].as<std::string, std::string>("");
	model->_mesh = MeshMgr::inst().req(key);
	if (!model->_mesh)
		return {};

	if (!model->initShader())
		return {};
	if (!model->initGL())
		return {};

	return model;
}

Model::~Model() {
	_mesh = nullptr;
	_shader = nullptr;

	if (_vao) {
		glDeleteVertexArrays(1, &_vao);
		_vao = 0;
	}
}

bool Model::initShader() {
	const std::string key = _conf["shader.name"].as<std::string, std::string>("");
	_shader = ShaderMgr::inst().req(key);
	if (!_shader) {
		std::cout << "model shader error, " << key << std::endl;
		return false;
	}

	_lpos = _shader->getVar("vt_pos");
	_lcolor = _shader->getVar("vt_color");
	_luv = _shader->getVar("vt_uv");
	_lnormal = _shader->getVar("vt_normal");
	_ltex = _shader->getVar("tex");

	const auto node = _conf["shader.vars"];
	if (node.IsDefined()) {
		if (!attrs.guessAttrs(node))
			return false;

		for (auto& it: attrs) {
			if (it.second.type() == typeid(std::string)) {
				it.second = TextureMgr::inst().req(std::any_cast<std::string&>(it.second));
			}
		}
	}

	return true;
}

bool Model::initGL() {
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);
	glBindBuffer(GL_ARRAY_BUFFER, _mesh->getVBO());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _mesh->getIBO());
	if (_lpos >= 0)
		glVertexAttribPointer(_lpos, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)0);
	if (_lcolor >= 0)
		glVertexAttribPointer(_lcolor, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	if (_luv >= 0)
		glVertexAttribPointer(_luv, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
	if (_lnormal >= 0)
		glVertexAttribPointer(_lnormal, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (void*)(8 * sizeof(GLfloat)));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return true;
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
	if (_lnormal >= 0)
		glEnableVertexAttribArray(_lnormal);
	if (_luv >= 0) {
		glEnableVertexAttribArray(_luv);
	}

	GLint loc{0};
	GLuint tex{0};
	for (const auto& it: attrs) {
		loc = _shader->getVar(it.first);
		if (loc < 0)
			continue;

		if (it.second.type() == typeid(float)) {
			glUniform1f(loc, std::any_cast<const float&>(it.second));
		}
		else if (it.second.type() == typeid(glm::vec3)) {
			glUniform3fv(loc, 1, glm::value_ptr(std::any_cast<const glm::vec3&>(it.second)));
		}
		else if (it.second.type() == typeid(Texture::ptr)) {
			glActiveTexture(GL_TEXTURE0 + tex);
			glBindTexture(GL_TEXTURE_2D, std::any_cast<const Texture::ptr&>(it.second)->getTexture());
			glUniform1i(loc, tex);
		}
		else {
			std::cout << "model var unknow, name: " << it.first << ", type: " << it.second.type().name() << std::endl;
		}
	}

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

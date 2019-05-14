#include "model.hpp"

ModelProto::ptr ModelProto::create(const std::string& name) {
	ModelProto::ptr model = ModelProto::ptr(new ModelProto());
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

	model->setName(name);
	return model;
}

ModelProto::~ModelProto() {
	_mesh = nullptr;
	_shader = nullptr;

	if (_vao) {
		glDeleteVertexArrays(1, &_vao);
		_vao = 0;
	}
}

bool ModelProto::initShader() {
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

	const auto node = _conf["shader.vars"];
	if (node.IsDefined()) {
		if (!attrs.guessAttrs(node))
			return false;

		for (auto& it: attrs) {
			if (it.second.type() == typeid(std::string)) {
				const auto& tex = TextureMgr::inst().req(std::any_cast<std::string&>(it.second));
				if (!tex)
					return false;
				it.second = tex;
			}
		}
	}

	return true;
}

bool ModelProto::initGL() {
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

void ModelProto::draw(const glm::mat4& view, const glm::mat4& proj) {
	_shader->useProgram();
	_shader->setVars(attrs);

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

	for (auto& it: _insts) {
		it.second->draw(_shader);
	}

	glBindVertexArray(0);
}

ModelInst::ptr ModelInst::create(const ModelProto::ptr& proto, const Config::node& conf) {
	ModelInst::ptr model = ModelInst::ptr(new ModelInst());
	
	glm::mat4 mat{1.0f};
	mat = glm::translate(mat, conf["pos"].as<glm::vec3>());
	const Config::node scale = conf["scale"];
	if (scale.IsDefined()) {
		mat = glm::scale(mat, glm::vec3(scale.as<float>()));
	}
	const Config::node rotate = conf["rotate"];
	if (rotate.IsDefined()) {
		mat = glm::rotate(mat, glm::radians(rotate[0].as<float>()), glm::vec3(0.0, 1.0, 0.0));
		mat = glm::rotate(mat, glm::radians(rotate[1].as<float>()), glm::vec3(1.0, 0.0, 0.0));
		mat = glm::rotate(mat, glm::radians(rotate[2].as<float>()), glm::vec3(0.0, 0.0, 1.0));
	}
	model->setMatrix(mat);
	model->setName(conf["name"].as<std::string>());
	model->_proto = proto;

	return model;
}

void ModelInst::draw(const Shader::ptr& shader) {
	shader->setVars(attrs);
	glUniformMatrix4fv(shader->getVar("model"), 1, GL_FALSE, glm::value_ptr(_mat));
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

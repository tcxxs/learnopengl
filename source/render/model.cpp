#include "model.hpp"

ModelProto::ptr ModelProto::create(const std::string& name) {
	ModelProto::ptr model = ModelProto::ptr(new ModelProto());
	model->setName(name);

	std::filesystem::path path = std::filesystem::current_path() / "resource" / "model" / (name + ".yml");
	if (!model->_conf.load(path)) {
		std::cout << "model config error, " << path << std::endl;
		return {};
	}

	if (!model->_loadAssimp())
		return {};
	if (!model->_initShader())
		return {};

	return model;
}

ModelProto::~ModelProto() {
}

bool ModelProto::_loadAssimp() {
	std::string name = _conf["file"].as<std::string>();
	std::filesystem::path path = std::filesystem::current_path() / "resource" / "model" / name;
	const aiScene* scene = _imp.ReadFile(path.string(), aiProcess_Triangulate | aiProcess_FlipUVs);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "model assimp error, " << _imp.GetErrorString() << std::endl;
		return false;
	}

	if (!_loadNode(path, scene->mRootNode, scene))
		return false;

	return true;
}

bool ModelProto::_loadNode(const std::filesystem::path& path, aiNode *node, const aiScene *scene) {
	for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
		aiMesh* ms = scene->mMeshes[node->mMeshes[i]];
		Mesh::ptr mesh = Mesh::create(path, ms, scene);
		if (!mesh)
			return false;
		_meshs.push_back(mesh);
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i) {
		if (!_loadNode(path, node->mChildren[i], scene))
			return false;
	}
	return true;
}

bool ModelProto::_initShader() {
	const std::string key = _conf["shader.name"].as<std::string, std::string>("");
	_shader = ShaderMgr::inst().req(key);
	if (!_shader) {
		std::cout << "model shader error, " << key << std::endl;
		return false;
	}

	const auto node = _conf["shader.vars"];
	if (node.IsDefined()) {
		if (!attrs.guessAttrs(node))
			return false;

		for (auto& it : attrs) {
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

void ModelProto::draw(const Camera::ptr& cam, const std::map<std::string, LightProto::ptr>& lights) {
	_shader->useProgram();
	_shader->setVars(attrs);

	glUniformMatrix4fv(_shader->getVar("view"), 1, GL_FALSE, glm::value_ptr(cam->getView()));
	glUniformMatrix4fv(_shader->getVar("proj"), 1, GL_FALSE, glm::value_ptr(cam->getProj()));
	_shader->setVar("camera_pos", cam->getPos());

	int dirs{0}, points{0}, spots{0};
	std::string light;
	for (const auto& it_proto : lights) {
		const LightProto::ptr& light_proto = it_proto.second;
		const std::string& light_type = light_proto->getName();
		for (const auto& it_inst : light_proto->container()) {
			if (light_type == "dir") {
				light = string_format("dirs[%d]", dirs++);
				_shader->setVar(light + ".dir", it_inst.second->getDir());
				_shader->setVar(light + ".ambient", light_proto->attrs.getAttr<glm::vec3>("ambient"));
				_shader->setVar(light + ".diffuse", light_proto->attrs.getAttr<glm::vec3>("diffuse"));
				_shader->setVar(light + ".specular", light_proto->attrs.getAttr<glm::vec3>("specular"));
			}
			else if (light_type == "point") {
				light = string_format("points[%d]", points++);
				_shader->setVar(light + ".pos", it_inst.second->getPos());
				_shader->setVar(light + ".ambient", light_proto->attrs.getAttr<glm::vec3>("ambient"));
				_shader->setVar(light + ".diffuse", light_proto->attrs.getAttr<glm::vec3>("diffuse"));
				_shader->setVar(light + ".specular", light_proto->attrs.getAttr<glm::vec3>("specular"));
				_shader->setVar(light + ".constant", light_proto->attrs.getAttr<float>("constant"));
				_shader->setVar(light + ".linear", light_proto->attrs.getAttr<float>("linear"));
				_shader->setVar(light + ".quadratic", light_proto->attrs.getAttr<float>("quadratic"));
			}
			else if (light_type == "spot") {
				light = string_format("spots[%d]", spots++);
				_shader->setVar(light + ".pos", it_inst.second->getPos());
				_shader->setVar(light + ".dir", it_inst.second->getDir());
				_shader->setVar(light + ".ambient", light_proto->attrs.getAttr<glm::vec3>("ambient"));
				_shader->setVar(light + ".diffuse", light_proto->attrs.getAttr<glm::vec3>("diffuse"));
				_shader->setVar(light + ".specular", light_proto->attrs.getAttr<glm::vec3>("specular"));
				_shader->setVar(light + ".constant", light_proto->attrs.getAttr<float>("constant"));
				_shader->setVar(light + ".linear", light_proto->attrs.getAttr<float>("linear"));
				_shader->setVar(light + ".quadratic", light_proto->attrs.getAttr<float>("quadratic"));
				_shader->setVar(light + ".inner", cos(glm::radians(light_proto->attrs.getAttr<float>("inner"))));
				_shader->setVar(light + ".outter", cos(glm::radians(light_proto->attrs.getAttr<float>("outter"))));
			}
		}
	}
	_shader->setVar("uses.dirs", dirs);
	_shader->setVar("uses.points", points);
	_shader->setVar("uses.spots", spots);

	for (auto& it : _insts) {
		it.second->draw(_shader);
		for (auto& itm : _meshs) {
			itm->draw();
		}
	}
}

ModelInst::ptr ModelInst::create(const ModelProto::ptr& proto, const Config::node& conf) {
	ModelInst::ptr model = ModelInst::ptr(new ModelInst());
	model->setName(conf["name"].as<std::string>());

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

	return model;
}

void ModelInst::draw(const Shader::ptr& shader) {
	shader->setVars(attrs);
	glUniformMatrix4fv(shader->getVar("model"), 1, GL_FALSE, glm::value_ptr(_mat));
}

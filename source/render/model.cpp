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

	return model;
}

bool ModelProto::_loadAssimp() {
	std::string name = _conf["file"].as<std::string>();
	std::filesystem::path path = std::filesystem::current_path() / "resource" / "model" / name;
	const aiScene* scene = _imp.ReadFile(path.string(), aiProcess_Triangulate);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "model assimp error, " << _imp.GetErrorString() << std::endl;
		return false;
	}

	if (!_loadNode(scene->mRootNode, scene))
		return false;

	return true;
}

bool ModelProto::_loadNode(aiNode* node, const aiScene* scene) {
	for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
		aiMesh* ms = scene->mMeshes[node->mMeshes[i]];
		MeshProto::ptr mesh = MeshProto::create(_conf, ms, scene);
		if (!mesh)
			return false;
		_meshs.push_back(mesh);
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i) {
		if (!_loadNode(node->mChildren[i], scene))
			return false;
	}
	return true;
}

void ModelProto::draw(const Camera::ptr& cam, const std::map<std::string, LightProto::ptr>& lights) {
	for (auto& it : _insts) {
		it.second->draw(cam, lights);
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

	const std::string shader = conf["shader"].as<std::string>("");
	for (const auto& it : proto->getMeshs()) {
		MeshInst::ptr mesh = it->instance(shader);
		if (!mesh)
			return {};
		model->_meshs.push_back(mesh);
	}

	return model;
}

void ModelInst::draw(const Camera::ptr& cam, const std::map<std::string, LightProto::ptr>& lights) {
	for (auto& it: _meshs) {
		it->draw(cam, lights, _mat, attrs);
	}
}

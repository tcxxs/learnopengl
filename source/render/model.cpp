#include "model.hpp"

ModelProto::ptr ModelProto::create(const std::string& name) {
	if (_confs.root().IsNull()) {
		std::filesystem::path path = std::filesystem::current_path() / "resource" / "models.yml";
		if (!_confs.load(path)) {
			std::cout << "models config error";
			return {};
		}
	}
	Config::node conf = _confs[name];
	if (!conf.IsDefined())
		return {};

	ModelProto::ptr model = ModelProto::ptr(new ModelProto());
	model->setName(name);

	if (conf["file"].IsScalar()) {
		if (!model->_loadAssimp(conf))
			return {};
	}
	else {
		if (!model->_loadVertex(conf))
			return {};
	}

	return model;
}

bool ModelProto::_loadAssimp(const Config::node& conf) {
	std::string name = conf["file"].as<std::string>();
	std::filesystem::path path = std::filesystem::current_path() / "resource" / "model" / name;
	const aiScene* scene = _imp.ReadFile(path.string(), aiProcess_Triangulate);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "model assimp error, " << _imp.GetErrorString() << std::endl;
		return false;
	}

	if (!_loadNode(conf, scene->mRootNode, scene))
		return false;

	return true;
}

bool ModelProto::_loadNode(const Config::node& conf, aiNode* node, const aiScene* scene) {
	for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
		aiMesh* ms = scene->mMeshes[node->mMeshes[i]];
		MeshProto::ptr mesh = MeshProto::create(conf, ms, scene);
		if (!mesh)
			return false;
		_meshs.push_back(mesh);
	}

	for (unsigned int i = 0; i < node->mNumChildren; ++i) {
		if (!_loadNode(conf, node->mChildren[i], scene))
			return false;
	}
	return true;
}

bool ModelProto::_loadVertex(const Config::node& conf) {
	MeshProto::ptr mesh = MeshProto::create(conf);
	if (!mesh)
		return false;
	_meshs.push_back(mesh);
	return true;
}

ModelInst::ptr ModelInst::create(const ModelProto::ptr& proto, const Config::node& conf) {
	ModelInst::ptr model = ModelInst::ptr(new ModelInst());
	model->setName(conf["name"].as<std::string>());

	glm::mat4 mat{1.0f};
	glm::vec3 pos = conf["pos"].as<glm::vec3>();
	mat = glm::translate(mat, pos);
	const Config::node scale = conf["scale"];
	if (scale.IsDefined()) {
		mat = glm::scale(mat, glm::vec3(scale.as<float>()));
	}
	const Config::node dir = conf["dir"];
	if (dir.IsDefined()) {
		glm::mat4 rot = glm::lookAt(pos, pos + dir.as<glm::vec3>(), glm::vec3(0, 1, 0));
		mat = mat * rot;
	}
	model->setMatrix(mat);

	const std::string material = conf["material"].as<std::string>("");
	for (const auto& it : proto->getMeshs()) {
		MeshInst::ptr mesh = it->instance(material);
		if (!mesh)
			return {};
		model->_meshs.push_back(mesh);
	}

	return model;
}

void ModelInst::draw(CommandQueue& cmds) {
	for (auto& it: _meshs) {
		it->draw(cmds, _mat, attrs);
	}
}

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

	const Config::node ins = conf["instance"];
	if (ins.IsDefined()) {
		for (const auto& it: ins) {
			model->_addInstance(it);
		}
		if (!model->_initInstance())
			return {};
	}
	else {
		model->_addInstance(conf);
	}

	const std::string material = conf["material"].as<std::string>("");
	for (const auto& it: proto->getMeshs()) {
		MeshInst::ptr mesh = it->instance();
		if (!mesh)
			return {};
		if (!mesh->changeMaterial(material))
			return {};
		model->_meshs.push_back(mesh);
	}

	return model;
}

ModelInst::~ModelInst() {
	if (_vbo) {
		glDeleteBuffers(1, &_vbo);
		_vbo = 0;
	}
}

void ModelInst::_addInstance(const Config::node& conf) {
	glm::mat4& mat = _mats.emplace_back(1.0f);
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
}

bool ModelInst::_initInstance() {
	size_t size = sizeof(glm::mat4);
	glCreateBuffers(1, &_vbo);
	glNamedBufferStorage(_vbo, _mats.size() * size, NULL, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);

	void* data = glMapNamedBuffer(_vbo, GL_WRITE_ONLY);
	for (int i = 0; i < _mats.size(); ++i) {
		memcpy((char*)data + i * size, glm::value_ptr(_mats[i]), size);
	}
	glUnmapNamedBuffer(_vbo);

	return true;
}

int ModelInst::draw(CommandQueue& cmds) {
	int total{0};
	int n;
	for (auto& it: _meshs) {
		n = it->draw(cmds);
		if (n < 0)
			return -1;
		total += n;

		Command& cmd = cmds.back();
		if (_vbo) {
			if (!cmd.material->getShader()->bindVertex(VERTEX_INSTANCE, it->getVAO(), _vbo))
				return -1;
			cmd.ins = (int)_mats.size();
		}
		else {
			cmd.model = _mats[0];
		}
		cmd.attrs.updateAttrs(attrs);
	}

	return total;
}

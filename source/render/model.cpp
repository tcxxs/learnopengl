#include "model.hpp"
#include "glm/gtx/transform.hpp"

ModelProto::ptr ModelProto::create(const std::string& name) {
	if (_confs.root().IsNull()) {
		std::filesystem::path path = std::filesystem::current_path() / "resource" / "models.yml";
		if (!_confs.load(path)) {
			std::cout << "models config error";
			return {};
		}
	}
	Config::node conf = _confs[name];
	if (!Config::valid(conf)) {
		std::printf("models config not find, %s", name.c_str());
		return {};
	}

	ModelProto::ptr model = std::make_shared<ModelProto>();
	model->setName(name);

	if (conf["file"].IsScalar()) {
		if (!model->_loadAssimp(conf))
			return {};
	}
	else {
		if (!model->_loadVertex(conf))
			return {};
	}

	if (!model->_initMaterial(conf))
		return {};

	return model;
}

bool ModelProto::_loadAssimp(const Config::node& conf) {
	std::string name = conf["file"].as<std::string>();
	std::filesystem::path path = std::filesystem::current_path() / "resource" / "model" / name;
	const aiScene* scene = _imp.ReadFile(path.string(), aiProcess_Triangulate | aiProcess_CalcTangentSpace);
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

bool ModelProto::_initMaterial(const Config::node& conf) {
	const std::string key = conf["material"].as<std::string>();
	_mate = MaterialMgr::inst().req(key);
	if (!_mate)
		return false;

	return true;
}

ModelInst::ptr ModelInst::create(const ModelProto::ptr& proto, const Config::node& conf) {
	ModelInst::ptr model = ModelInst::ptr(new ModelInst());
	model->_proto = proto;
	if (Config::valid(conf["name"]))
		model->setName(conf["name"].as<std::string>());

	if (!model->_initMaterial(conf))
		return {};

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

	for (const auto& it: proto->getMeshs()) {
		MeshInst::ptr mesh = it->instance();
		if (!mesh)
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
	
	std::any val = Config::guess(conf["pos"]);
	glm::vec3 pos = std::any_cast<glm::vec3>(val);
	mat = glm::translate(mat, pos);

	const Config::node scale = conf["scale"];
	if (scale.IsDefined()) {
		mat = glm::scale(mat, glm::vec3(scale.as<float>()));
	}
	
	const Config::node rotate = conf["rotate"];
	if (rotate.IsDefined()) {
		float x = rotate[0].as<float>();
		float y = rotate[1].as<float>();
		float z = rotate[2].as<float>();
		mat *= glm::rotate(glm::radians(x), glm::vec3(1.0, 0.0, 0.0));
		mat *= glm::rotate(glm::radians(y), glm::vec3(0.0, 1.0, 0.0));
		mat *= glm::rotate(glm::radians(z), glm::vec3(0.0, 0.0, 1.0));
	}
}

bool ModelInst::_initMaterial(const Config::node& conf) {
	if (Config::valid(conf["material"])) {
		const std::string& name = conf["material"].as<std::string>();
		_mate = MaterialMgr::inst().req(name);
		if (!_mate)
			return false;
	}
	else {
		_mate = _proto->getMaterial();
	}

	if (Config::valid(conf["materials"])) {
		for (const auto& it: conf["materials"]) {
			const std::string& pass = it.first.as<std::string>();
			const std::string& name = it.second.as<std::string>();
			Material::ptr mate = MaterialMgr::inst().req(name);
			if (!mate)
				return false;
			_mates[pass] = mate;
		}
	}

	return true;
}

bool ModelInst::_initInstance() {
	size_t size = sizeof(glm::mat4);
	glCreateBuffers(1, &_vbo);
	glNamedBufferStorage(_vbo, _mats.size() * size, nullptr, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);

	void* data = glMapNamedBuffer(_vbo, GL_WRITE_ONLY);
	for (int i = 0; i < _mats.size(); ++i) {
		memcpy((char*)data + i * size, glm::value_ptr(_mats[i]), size);
	}
	glUnmapNamedBuffer(_vbo);

	return true;
}

int ModelInst::draw(CommandQueue& cmds, const std::string& pass, const std::set<Shader::ptr>& shaders) {
	Material::ptr mate = _mate;
	const auto& find = _mates.find(pass);
	if (find != _mates.end())
		mate = find->second;
	if (shaders.count(mate->getShader()) <= 0)
		return 0;

	bool changed = false;
	if (_mateid != mate->getID()) {
		changed = true;
		_mateid = mate->getID();
	}

	int total{0};
	int n;
	for (auto& it: _meshs) {
		if (changed)
			it->changeMaterial(mate);
		n = it->draw(cmds);
		if (n < 0)
			return -1;
		total += n;

		Command& cmd = cmds.back();
		if (_vbo) {
			if (changed) {
				if (!mate->getShader()->bindVertex(VERTEX_INSTANCE, it->getVAO(), _vbo))
					return -1;
			}
			cmd.ins = (int)_mats.size();
		}
		else {
			cmd.model = _mats[0];
		}
		cmd.material = mate;
		cmd.attrs.updateAttrs(attrs);
	}

	return total;
}

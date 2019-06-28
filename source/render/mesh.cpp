#include "model.hpp"

MeshProto::ptr MeshProto::create(const Config::node& conf) {
	MeshProto::ptr mesh = std::shared_ptr<MeshProto>(new MeshProto());

	if (!mesh->_loadRaw(conf["file"]))
		return {};
	if (!mesh->_initGL())
		return {};
	if (!mesh->_initMaterial(conf["materials"]))
		return {};

	return mesh;
}

MeshProto::ptr MeshProto::create(const Config::node& conf, const aiMesh* ms, const aiScene* scene) {
	MeshProto::ptr mesh = std::shared_ptr<MeshProto>(new MeshProto());
	// mesh->setName(name);

	if (!mesh->_loadVertex(ms))
		return {};
	std::filesystem::path path = std::filesystem::current_path() / "resource" / "model" / conf["file"].as<std::string>();
	if (!mesh->_loadMaterial(path.parent_path(), ms, scene))
		return {};
	if (!mesh->_initGL())
		return {};
	if (!mesh->_initMaterial(conf["materials"]))
		return {};

	return mesh;
}

MeshProto::~MeshProto() {
	if (_vao) {
		glDeleteVertexArrays(1, &_vao);
		_vao = 0;
	}
	if (_vbo) {
		glDeleteBuffers(1, &_vbo);
		_vbo = 0;
	}
	if (_ibo) {
		glDeleteBuffers(1, &_ibo);
		_ibo = 0;
	}
}

bool MeshProto::_loadRaw(const Config::node& conf) {
	_pos = conf["layout"][0].as<unsigned int>(0);
	_uv = conf["layout"][1].as<unsigned int>(0);
	_normal = conf["layout"][2].as<unsigned int>(0);

	Config::node verts = conf["vertex"];
	_count = (unsigned int)verts.size() / (_pos + _uv + _normal);
	_verts.resize(verts.size(), 0.0f);
	int i = 0;
	for (const auto& it: conf["vertex"]) {
		_verts[i] = it.as<float>();
		++i;
	}

	return true;
}

bool MeshProto::_loadVertex(const aiMesh* mesh) {
	_count = mesh->mNumVertices;
	_pos = 3;
	if (mesh->mTextureCoords[0]) {
		_uv = 2;
	}
	if (mesh->mNormals) {
		_normal = 3;
	}

	int layout = (_pos + _uv + _normal);
	_verts.resize(_count * layout, 0.0f);
	for (unsigned int i = 0; i < _count; ++i) {
		_verts[(i * layout)] = mesh->mVertices[i].x;
		_verts[(i * layout) + 1] = mesh->mVertices[i].y;
		_verts[(i * layout) + 2] = mesh->mVertices[i].z;
		if (_uv > 0) {
			_verts[(i * layout) + _pos] = mesh->mTextureCoords[0][i].x;
			_verts[(i * layout) + _pos + 1] = mesh->mTextureCoords[0][i].y;
		}
		if (_normal > 0) {
			_verts[(i * layout) + _pos + _uv] = mesh->mNormals[i].x;
			_verts[(i * layout) + _pos + _uv + 1] = mesh->mNormals[i].y;
			_verts[(i * layout) + _pos + _uv + 2] = mesh->mNormals[i].z;
		}
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
		aiFace face = mesh->mFaces[i];
		_inds.insert(_inds.end(), face.mIndices, face.mIndices + face.mNumIndices);
	}

	return true;
}

bool MeshProto::_loadMaterial(const std::filesystem::path& path, const aiMesh* mesh, const aiScene* scene) {
	aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
	if (!_loadTexture(path, mat, aiTextureType_DIFFUSE, "material.diffuse"))
		return false;
	if (!_loadTexture(path, mat, aiTextureType_SPECULAR, "material.specular"))
		return false;

	return true;
}

bool MeshProto::_loadTexture(const std::filesystem::path& path, const aiMaterial* mat, const aiTextureType type, const std::string& name) {
	for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i) {
		aiString str;
		mat->GetTexture(type, i, &str);
		std::filesystem::path file = path / str.C_Str();

		Texture::ptr tex = TextureMgr::inst().req(file.string(), file);
		if (!tex)
			return false;
		attrs.setAttr(name, tex);
		break;
	}

	return true;
}

bool MeshProto::_initGL() {
	glGenVertexArrays(1, &_vao);
	glGenBuffers(1, &_vbo);
	glGenBuffers(1, &_ibo);

	glBindVertexArray(_vao);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, _verts.size() * sizeof(float), _verts.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _inds.size() * sizeof(GLuint), _inds.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(POS_LOC);
	glVertexAttribPointer(POS_LOC, _pos, GL_FLOAT, GL_FALSE, (_pos + _uv + _normal) * sizeof(float), (void*)0);
	if (_uv > 0) {
		glEnableVertexAttribArray(UV_LOC);
		glVertexAttribPointer(UV_LOC, _uv, GL_FLOAT, GL_FALSE, (_pos + _uv + _normal) * sizeof(float), (void*)(_pos * sizeof(float)));
	}
	if (_normal > 0) {
		glEnableVertexAttribArray(NORMAL_LOC);
		glVertexAttribPointer(NORMAL_LOC, _normal, GL_FLOAT, GL_FALSE, (_pos + _uv + _normal) * sizeof(float), (void*)((_pos + _uv) * sizeof(float)));
	}

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	if (oglError())
		return false;

	return true;
}

bool MeshProto::_initMaterial(const Config::node& conf) {
	for (const auto it: conf) {
		const std::string key = it.as<std::string>();
		Material::ptr mate = MaterialMgr::inst().req(key);
		if (!mate)
			return false;
		_materials[key] = mate;
	}

	return true;
}

void MeshProto::draw(CommandQueue& cmds,
                     const glm::mat4& model,
                     const Attributes& mattrs,
                     const Material::ptr& mate) {
	Command& cmd = cmds.emplace_back();
	cmd.vao = _vao;
	cmd.ibosize = (GLsizei)_inds.size();
	cmd.arrsize = (GLsizei)_count;
	cmd.model = model;
	cmd.material = mate;
	cmd.attrs.updateAttrs(attrs);
	cmd.attrs.updateAttrs(mattrs);
}

MeshInst::ptr MeshInst::create(const MeshProto::ptr& proto, const std::string& mate) {
	MeshInst::ptr mesh = std::shared_ptr<MeshInst>(new MeshInst());

	if (mate.empty()) {
		mesh->_material = proto->getMaterialDefault();
	}
	else {
		mesh->_material = proto->getMaterial(mate);
		if (!mesh->_material) {
			std::cout << "mesh instance, not found material, " << mate << std::endl;
			return {};
		}
	}
	return mesh;
}
#include "model.hpp"

MeshProto::ptr MeshProto::create(const Config::node& conf) {
	MeshProto::ptr mesh = std::shared_ptr<MeshProto>(new MeshProto());

	if (!mesh->_loadRaw(conf["file"]))
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
	if (!mesh->_initMaterial(conf["materials"]))
		return {};

	return mesh;
}

MeshProto::~MeshProto() {
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
	int layout = _pos + _uv + _normal;

	Config::node vconf = conf["vertex"];
	_vsize = (unsigned int)vconf.size() / layout;
	std::vector<VertexBase> verts(_vsize);
	for (int i = 0; i < _vsize; ++i) {
		if (_pos > 0) {
			verts[i].pos[0] = vconf[i * layout].as<float>();
			verts[i].pos[1] = vconf[i * layout + 1].as<float>();
			verts[i].pos[2] = vconf[i * layout + 2].as<float>();
		}
		if (_uv > 0) {
			verts[i].uv[0] = vconf[i * layout + _pos].as<float>();
			verts[i].uv[1] = vconf[i * layout + _pos + 1].as<float>();
		}
		if (_normal > 0) {
			verts[i].normal[0] = vconf[i * layout + _pos + _uv].as<float>();
			verts[i].normal[1] = vconf[i * layout + _pos + _uv + 1].as<float>();
			verts[i].normal[2] = vconf[i * layout + _pos + _uv + 2].as<float>();
		}
	}

	glCreateBuffers(1, &_vbo);
	glNamedBufferStorage(_vbo, verts.size() * sizeof(VertexInstance), verts.data(), GL_DYNAMIC_STORAGE_BIT);

	return true;
}

bool MeshProto::_loadVertex(const aiMesh* mesh) {
	_pos = 3;
	if (mesh->mTextureCoords[0]) {
		_uv = 2;
	}
	if (mesh->mNormals) {
		_normal = 3;
	}
	int layout = (_pos + _uv + _normal);

	_vsize = mesh->mNumVertices;
	std::vector<VertexBase> verts(_vsize);
	for (int i = 0; i < _vsize; ++i) {
		verts[i].pos[0] = mesh->mVertices[i].x;
		verts[i].pos[1] = mesh->mVertices[i].y;
		verts[i].pos[2] = mesh->mVertices[i].z;
		if (_uv > 0) {
			verts[i].uv[0] = mesh->mTextureCoords[0][i].x;
			verts[i].uv[1] = mesh->mTextureCoords[0][i].y;
		}
		if (_normal > 0) {
			verts[i].normal[0] = mesh->mNormals[i].x;
			verts[i].normal[1] = mesh->mNormals[i].y;
			verts[i].normal[2] = mesh->mNormals[i].z;
		}
	}

	glCreateBuffers(1, &_vbo);
	glNamedBufferStorage(_vbo, verts.size() * sizeof(VertexInstance), verts.data(), GL_DYNAMIC_STORAGE_BIT);

	std::vector<GLuint> inds;
	for (int i = 0; i < mesh->mNumFaces; ++i) {
		aiFace face = mesh->mFaces[i];
		inds.insert(inds.end(), face.mIndices, face.mIndices + face.mNumIndices);
	}
	_isize = inds.size();
	glCreateBuffers(1, &_ibo);
	glNamedBufferStorage(_ibo, inds.size() * sizeof(GLuint), inds.data(), GL_DYNAMIC_STORAGE_BIT);
	
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

MeshInst::ptr MeshInst::create(const MeshProto::ptr& proto) {
	MeshInst::ptr mesh = std::shared_ptr<MeshInst>(new MeshInst());

	return mesh;
}

 MeshInst::~MeshInst() {
	 if (_vao) {
		 glDeleteVertexArrays(1, &_vao);
		 _vao = 0;
	 }
}

bool MeshInst::changeMaterial(const std::string& mate) {
	if (mate.empty()) {
		_material = _proto->getMaterialDefault();
	}
	else {
		_material = _proto->getMaterial(mate);
		if (!_material) {
			std::cout << "mesh instance, not found material, " << mate << std::endl;
			return false;
		}
	}

	if (_vao) {
		glDeleteVertexArrays(1, &_vao);
		_vao = 0;
	}
	glCreateVertexArrays(1, &_vao);
	if (!_material->getShader()->bindVertex(VERTEX_BASE, _vao, _proto->getVBO())) {
		std::cout << "mesh change material, bind vertex base, " << mate << std::endl;
		return false;
	}
	glVertexArrayElementBuffer(_vao, _proto->getIBO());
	return true;
}

int MeshInst::draw(CommandQueue& cmds) {
	Command& cmd = cmds.emplace_back();
	cmd.vao = _vao;
	cmd.inds = _proto->getInds();
	cmd.verts = _proto->getVerts();
	cmd.material = _material;
	cmd.attrs.updateAttrs(_proto->attrs);
	return 1;
}

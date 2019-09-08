#include "model.hpp"

MeshProto::ptr MeshProto::create(const Config::node& conf) {
	MeshProto::ptr mesh = std::make_shared<MeshProto>();

	if (!mesh->_loadRaw(conf["file"]))
		return {};

	return mesh;
}

MeshProto::ptr MeshProto::create(const Config::node& conf, const aiMesh* ms, const aiScene* scene) {
	MeshProto::ptr mesh = std::make_shared<MeshProto>();
	// mesh->setName(name);

	if (!mesh->_loadVertex(ms))
		return {};
	std::filesystem::path path = std::filesystem::current_path() / "resource" / "model" / conf["file"].as<std::string>();
	if (!mesh->_loadMaterial(path.parent_path(), ms, scene))
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
			verts[i].pos.x = vconf[i * layout].as<float>();
			verts[i].pos.y = vconf[i * layout + 1].as<float>();
			verts[i].pos.z = vconf[i * layout + 2].as<float>();
		}
		if (_uv > 0) {
			verts[i].uv.x = vconf[i * layout + _pos].as<float>();
			verts[i].uv.y = vconf[i * layout + _pos + 1].as<float>();
		}
		if (_normal > 0) {
			verts[i].normal.x = vconf[i * layout + _pos + _uv].as<float>();
			verts[i].normal.y = vconf[i * layout + _pos + _uv + 1].as<float>();
			verts[i].normal.z = vconf[i * layout + _pos + _uv + 2].as<float>();
		}
	}

	for (int i = 0; i < _vsize / 3; ++i) {
		const VertexBase& v1 = verts[i * 3];
		const VertexBase& v2 = verts[i * 3 + 1];
		const VertexBase& v3 = verts[i * 3 + 2];
		glm::vec3 e1 = v2.pos - v1.pos;
		glm::vec3 e2 = v3.pos - v1.pos;
		glm::vec2 d1 = v2.uv - v1.uv;
		glm::vec2 d2 = v3.uv - v1.uv;

		GLfloat f = 1.0f / (d1.x * d2.y - d2.x * d1.y);
		glm::vec3 tangent{0.0f};
		tangent.x = f * (d2.y * e1.x - d1.y * e2.x);
		tangent.y = f * (d2.y * e1.y - d1.y * e2.y);
		tangent.z = f * (d2.y * e1.z - d1.y * e2.z);
		tangent = glm::normalize(tangent);

		for (int j = 0; j < 3; ++j) {
			verts[i * 3 + j].tangent = tangent;
		}
	}

	glCreateBuffers(1, &_vbo);
	glNamedBufferStorage(_vbo, verts.size() * sizeof(VertexBase), verts.data(), GL_DYNAMIC_STORAGE_BIT);

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
		verts[i].pos.x = mesh->mVertices[i].x;
		verts[i].pos.y = mesh->mVertices[i].y;
		verts[i].pos.z = mesh->mVertices[i].z;
		verts[i].tangent.x = mesh->mTangents[i].x;
		verts[i].tangent.y = mesh->mTangents[i].y;
		verts[i].tangent.z = mesh->mTangents[i].z;
		if (_uv > 0) {
			verts[i].uv.x = mesh->mTextureCoords[0][i].x;
			verts[i].uv.y = mesh->mTextureCoords[0][i].y;
		}
		if (_normal > 0) {
			verts[i].normal.x = mesh->mNormals[i].x;
			verts[i].normal.y = mesh->mNormals[i].y;
			verts[i].normal.z = mesh->mNormals[i].z;
		}
	}

	std::vector<GLuint> inds;
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
		aiFace face = mesh->mFaces[i];
		inds.insert(inds.end(), face.mIndices, face.mIndices + face.mNumIndices);
	}
	_isize = (int)inds.size();

	glCreateBuffers(1, &_vbo);
	glNamedBufferStorage(_vbo, verts.size() * sizeof(VertexBase), verts.data(), GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);

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
	if (!_loadTexture(path, mat, aiTextureType_HEIGHT, "material.normal"))
		return false;
	if (!_loadTexture(path, mat, aiTextureType_DISPLACEMENT, "material.displace"))
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
		attrs.setAttr(name, tex->getValue());
		break;
	}

	return true;
}

MeshInst::ptr MeshInst::create(const MeshProto::ptr& proto) {
	MeshInst::ptr mesh = std::make_shared<MeshInst>();

	return mesh;
}

MeshInst::~MeshInst() {
	if (_vao) {
		glDeleteVertexArrays(1, &_vao);
		_vao = 0;
	}
}

bool MeshInst::changeMaterial(const Material::ptr& mate) {
	if (_vao) {
		glDeleteVertexArrays(1, &_vao);
		_vao = 0;
	}
	glCreateVertexArrays(1, &_vao);
	if (!mate->getShader()->bindVertex(VERTEX_BASE, _vao, _proto->getVBO())) {
		ERR("mesh change material, bind vertex base, %s", mate->getName().c_str());
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
	cmd.attrs.updateAttrs(_proto->attrs);
	return 1;
}

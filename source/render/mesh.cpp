#include "model.hpp"

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

bool MeshProto::_loadVertex(const aiMesh* mesh) {
	if (!mesh->mTextureCoords[0]) {
		std::cout << "mesh create, no texture uv" << std::endl;
		return false;
	}
	if (!mesh->mNormals) {
		std::cout << "mesh create, no normal" << std::endl;
		return false;
	}

	_verts.resize(mesh->mNumVertices, {glm::vec3(0.0f), glm::vec2(0.0f), glm::vec3(0.0f)});
	for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
		_verts[i].pos.x = mesh->mVertices[i].x;
		_verts[i].pos.y = mesh->mVertices[i].y;
		_verts[i].pos.z = mesh->mVertices[i].z;
		_verts[i].uv.x = mesh->mTextureCoords[0][i].x;
		_verts[i].uv.y = mesh->mTextureCoords[0][i].y;
		_verts[i].normal.x = mesh->mNormals[i].x;
		_verts[i].normal.y = mesh->mNormals[i].y;
		_verts[i].normal.z = mesh->mNormals[i].z;
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
	glBufferData(GL_ARRAY_BUFFER, _verts.size() * sizeof(Vertex), _verts.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _inds.size() * sizeof(GLuint), _inds.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(POS_LOC);
	glVertexAttribPointer(POS_LOC, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
	glEnableVertexAttribArray(UV_LOC);
	glVertexAttribPointer(UV_LOC, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glEnableVertexAttribArray(NORMAL_LOC);
	glVertexAttribPointer(NORMAL_LOC, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

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
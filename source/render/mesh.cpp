#include "model.hpp"

MeshProto::ptr MeshProto::create(const Config& conf, const aiMesh* ms, const aiScene* scene) {
	MeshProto::ptr mesh = std::shared_ptr<MeshProto>(new MeshProto());
	// mesh->setName(name);

	if (!mesh->_loadVertex(ms))
		return {};
	std::filesystem::path path = std::filesystem::current_path() / "resource" / "model" / conf["file"].as<std::string>();
	if (!mesh->_loadMaterial(path.parent_path(), ms, scene))
		return {};
	if (!mesh->_initGL())
		return {};
	if (!mesh->_initShader(conf["materials"]))
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

bool MeshProto::_initShader(const Config::node& conf) {
	for (const auto it : conf) {
		const std::string key = it["name"].as<std::string>();
		const ShaderProto::ptr& proto = ShaderProtoMgr::inst().req(key);
		const ShaderInst::ptr& shader = proto->instance();
		if (!shader) {
			std::cout << "model shader error, " << key << std::endl;
			return false;
		}

		const auto node = it["vars"];
		if (node.IsDefined()) {
			if (!shader->attrs.updateConf(node))
				return false;
			for (auto& it : shader->attrs) {
				if (it.second.type() == typeid(std::string)) {
					const auto& tex = TextureMgr::inst().req(std::any_cast<std::string&>(it.second));
					if (!tex)
						return false;
					it.second = tex;
				}
			}
		}

		_shaders[proto->getName()] = shader;
	}

	return true;
}

void MeshProto::draw(const Camera::ptr& cam,
                     const std::map<std::string, LightProto::ptr>& lights,
                     const glm::mat4& model,
                     const Attributes& mattrs,
                     const ShaderInst::ptr& shader) {
	shader->useProgram();
	shader->setVars(attrs);
	shader->setVars(mattrs);

	glUniformMatrix4fv(shader->getVar("view"), 1, GL_FALSE, glm::value_ptr(cam->getView()));
	glUniformMatrix4fv(shader->getVar("proj"), 1, GL_FALSE, glm::value_ptr(cam->getProj()));
	glUniformMatrix4fv(shader->getVar("model"), 1, GL_FALSE, glm::value_ptr(model));
	shader->setVar("camera_pos", cam->getPos());

	int dirs{0}, points{0}, spots{0};
	std::string light;
	for (const auto& it_proto : lights) {
		const LightProto::ptr& light_proto = it_proto.second;
		const std::string& light_type = light_proto->getName();
		for (const auto& it_inst : light_proto->container()) {
			if (light_type == "dir") {
				light = string_format("dirs[%d]", dirs++);
				shader->setVar(light + ".dir", it_inst.second->getDir());
				shader->setVar(light + ".ambient", light_proto->attrs.getAttr<glm::vec3>("ambient"));
				shader->setVar(light + ".diffuse", light_proto->attrs.getAttr<glm::vec3>("diffuse"));
				shader->setVar(light + ".specular", light_proto->attrs.getAttr<glm::vec3>("specular"));
			}
			else if (light_type == "point") {
				light = string_format("points[%d]", points++);
				shader->setVar(light + ".pos", it_inst.second->getPos());
				shader->setVar(light + ".ambient", light_proto->attrs.getAttr<glm::vec3>("ambient"));
				shader->setVar(light + ".diffuse", light_proto->attrs.getAttr<glm::vec3>("diffuse"));
				shader->setVar(light + ".specular", light_proto->attrs.getAttr<glm::vec3>("specular"));
				shader->setVar(light + ".constant", light_proto->attrs.getAttr<float>("constant"));
				shader->setVar(light + ".linear", light_proto->attrs.getAttr<float>("linear"));
				shader->setVar(light + ".quadratic", light_proto->attrs.getAttr<float>("quadratic"));
			}
			else if (light_type == "spot") {
				light = string_format("spots[%d]", spots++);
				shader->setVar(light + ".pos", it_inst.second->getPos());
				shader->setVar(light + ".dir", it_inst.second->getDir());
				shader->setVar(light + ".ambient", light_proto->attrs.getAttr<glm::vec3>("ambient"));
				shader->setVar(light + ".diffuse", light_proto->attrs.getAttr<glm::vec3>("diffuse"));
				shader->setVar(light + ".specular", light_proto->attrs.getAttr<glm::vec3>("specular"));
				shader->setVar(light + ".constant", light_proto->attrs.getAttr<float>("constant"));
				shader->setVar(light + ".linear", light_proto->attrs.getAttr<float>("linear"));
				shader->setVar(light + ".quadratic", light_proto->attrs.getAttr<float>("quadratic"));
				shader->setVar(light + ".inner", cos(glm::radians(light_proto->attrs.getAttr<float>("inner"))));
				shader->setVar(light + ".outter", cos(glm::radians(light_proto->attrs.getAttr<float>("outter"))));
			}
		}
	}
	shader->setVar("uses.dirs", dirs);
	shader->setVar("uses.points", points);
	shader->setVar("uses.spots", spots);

	glBindVertexArray(_vao);
	glDrawElements(GL_TRIANGLES, (GLsizei)_inds.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

MeshInst::ptr MeshInst::create(const MeshProto::ptr& proto, const std::string& shader) {
	MeshInst::ptr mesh = std::shared_ptr<MeshInst>(new MeshInst());

	if (shader.empty()) {
		mesh->_shader = proto->getShaderDefault();
	}
	else {
		mesh->_shader = proto->getShader(shader);
		if (!mesh->_shader) {
			std::cout << "mesh instance, not found shader, " << shader << std::endl;
			return {};
		}
	}
	return mesh;
}
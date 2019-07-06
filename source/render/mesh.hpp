#pragma once

#include "glad/glad.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "utils/pattern.hpp"
#include "utils/utils.hpp"
#include "utils/resource.hpp"
#include "render/material.hpp"
#include "render/camera.hpp"
#include "render/light.hpp"
#include "render/command.hpp"

class MeshProto;
class MeshInst: public ResInst<MeshProto, MeshInst> {
public:
	static ptr create(const proto_ptr& proto);
	virtual ~MeshInst();

	inline const GLuint getVAO() const { return _vao; }

	bool changeMaterial(const std::string& mate);
	int draw(CommandQueue& cmds);

private:
	GLuint _vao{0}, _ins{0};
	Material::ptr _material;
};

class MeshProto: public ResProto<MeshProto, MeshInst> {
public:
	static ptr create(const Config::node& conf);
	static ptr create(const Config::node& conf, const aiMesh* ms, const aiScene* scene);
	virtual ~MeshProto();

	inline const GLuint getVBO() const { return _vbo; }
	inline const GLuint getIBO() const { return _ibo; }
	inline const int getVerts() const { return _vsize; }
	inline const int getInds() const { return _isize; }

	inline const Material::ptr& getMaterial(const std::string& name) const {
		const auto& it = _materials.find(name);
		if (it == _materials.end())
			return Material::empty;
		else
			return it->second;
	}
	inline const Material::ptr& getMaterialDefault() const {
		return _materials.begin()->second;
	}

protected:
	bool _loadRaw(const Config::node& conf);
	bool _loadVertex(const aiMesh* mesh);
	bool _loadMaterial(const std::filesystem::path& path, const aiMesh* mesh, const aiScene* scene);
	bool _loadTexture(const std::filesystem::path& path, const aiMaterial* mat, const aiTextureType type, const std::string& name);
	bool _initMaterial(const Config::node& conf);

public:
	Attributes attrs;

private:
	int _pos{0}, _uv{0}, _normal{0};
	int _vsize{0}, _isize{0};
	GLuint _vbo{0}, _ibo{0};
	std::map<std::string, Material::ptr> _materials;
};

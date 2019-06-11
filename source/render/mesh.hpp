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

struct Vertex {
	glm::vec3 pos;
	glm::vec2 uv;
	glm::vec3 normal;
};

class MeshProto;
class MeshInst: public ResInst<MeshProto, MeshInst> {
public:
	static ptr create(const proto_ptr& proto, const std::string& mate);

	inline void draw(CommandQueue& cmds,
	                 const glm::mat4& model,
	                 const Attributes& mattrs);

private:
	Material::ptr _material;
};

class MeshProto: public ResProto<MeshProto, MeshInst> {
public:
	static ptr create(const Config& conf, const aiMesh* ms, const aiScene* scene);
	virtual ~MeshProto();

	inline const GLuint getVAO() const { return _vao; }
	inline const GLuint getVBO() const { return _vbo; }
	inline const GLuint getIBO() const { return _ibo; }

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

	void draw(CommandQueue& cmds,
	          const glm::mat4& model,
	          const Attributes& mattrs,
	          const Material::ptr& mate);

protected:
	bool _loadVertex(const aiMesh* mesh);
	bool _loadMaterial(const std::filesystem::path& path, const aiMesh* mesh, const aiScene* scene);
	bool _loadTexture(const std::filesystem::path& path, const aiMaterial* mat, const aiTextureType type, const std::string& name);
	bool _initGL();
	bool _initShader(const Config::node& conf);

public:
	Attributes attrs;

private:
	std::vector<Vertex> _verts;
	std::vector<GLuint> _inds;
	GLuint _vao{0}, _vbo{0}, _ibo{0};
	std::map<std::string, Material::ptr> _materials;
};

inline void MeshInst::draw(CommandQueue& cmds,
                           const glm::mat4& model,
                           const Attributes& mattrs) {
	_proto->draw(cmds, model, mattrs, _material);
}

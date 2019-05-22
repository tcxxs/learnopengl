#pragma once

#include "glad/glad.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "utils/pattern.hpp"
#include "utils/utils.hpp"
#include "utils/resource.hpp"

struct Vertex {
    glm::vec3 pos;
    glm::vec2 uv;
    glm::vec3 normal;
};

class Mesh: public Res<Mesh> {
public:
	static ptr create(const std::filesystem::path& path, const aiMesh *ms, const aiScene *scene);
	virtual ~Mesh();

	inline const GLuint getVAO() const { return _vao; }
	inline const GLuint getVBO() const { return _vbo; }
	inline const GLuint getIBO() const { return _ibo; }

	void draw();
protected:
	bool _loadVertex(const aiMesh* mesh);
	bool _loadMaterial(const std::filesystem::path& path, const aiMesh* mesh, const aiScene *scene);
	bool _loadTexture(const std::filesystem::path& path, const aiMaterial *mat, const aiTextureType type, const std::string& name);

public:
	Attributes attrs;
private:
	std::vector<Vertex> _verts;
	std::vector<GLuint> _inds;
	GLuint _vao{0}, _vbo{0}, _ibo{0};
};

using MeshMgr = ResMgr<Mesh>;
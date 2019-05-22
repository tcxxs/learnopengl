#pragma once

#include "glad/glad.h"
#include "utils/pattern.hpp"
#include "utils/utils.hpp"
#include "utils/resource.hpp"

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
};

class Mesh: public Res<Mesh> {
public:
	static ptr create(const std::string& name, const std::vector<GLfloat>& verts, const std::vector<GLuint>& inds);
	static ptr create(const std::string& name) { return {}; };
	virtual ~Mesh();

	inline const GLuint getVAO() const { return _vao; }
	inline const GLuint getVBO() const { return _vbo; }
	inline const GLuint getIBO() const { return _ibo; }

private:
	std::vector<Vertex> _verts;
	std::vector<GLuint> _inds;
	GLuint _vao{0}, _vbo{0}, _ibo{0};
};

using MeshMgr = ResMgr<Mesh>;
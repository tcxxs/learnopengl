#pragma once

#include "glad/glad.h"
#include "utils/pattern.hpp"
#include "utils/utils.hpp"
#include "utils/resource.hpp"

class Mesh: public Res<Mesh> {
public:
	static ptr create(const std::vector<GLfloat>& verts, const std::vector<GLuint>& inds);
	static ptr create(const std::string& name) { return {}; };
	virtual ~Mesh();

	inline const GLuint getVBO() const { return _vbo; }
	inline const GLuint getIBO() const { return _ibo; }

private:
	GLuint _vbo{0};
	GLuint _ibo{0};
};

using MeshMgr = ResMgr<std::string, Mesh>;
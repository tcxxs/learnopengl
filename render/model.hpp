#pragma once

#include "glad/glad.h"
#include "event.hpp"
#include "utils/resource.hpp"
#include "render/shader.hpp"

class Mesh: public Res<Mesh> {
public:
	static ptr create(const std::vector<GLfloat>& verts, const std::vector<GLuint>& inds);
	virtual ~Mesh();

	inline GLuint getVAO() const { return _vao; }

private:
	GLuint _vbo{0};
	GLuint _ibo{0};
	GLuint _vao{0};
};

class Model: public Res<Model> {
public:
	static ptr create(const Mesh::ptr& _mesh, const Shader::ptr& _shader);
	virtual ~Model();

	void draw();

private:
	Mesh::ptr _mesh;
	Shader::ptr _shader;
};

using ModelMgr = ResMgr<std::string, Model>;
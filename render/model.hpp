#pragma once

#include "glad/glad.h"
#include "event.hpp"
#include "utils/resource.hpp"
#include "render/shader.hpp"
#include "render/texture.hpp"

class Mesh: public Res<Mesh> {
public:
	static ptr create(const std::vector<GLfloat>& verts, const std::vector<GLuint>& inds);
	virtual ~Mesh();

	inline const GLuint getVBO() const { return _vbo; }
	inline const GLuint getIBO() const { return _ibo; }

private:
	GLuint _vbo{0};
	GLuint _ibo{0};
};

class Model: public Res<Model> {
public:
	static ptr create(const Mesh::ptr& mesh, const Shader::ptr& shader, const Texture::ptr& tex);
	virtual ~Model();

	void draw();

private:
	Mesh::ptr _mesh;
	Shader::ptr _shader;
	Texture::ptr _tex;
	GLuint _vao{0};
	GLint _pos{0}, _color{0}, _uv{0};
};

using ModelMgr = ResMgr<std::string, Model>;
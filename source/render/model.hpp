#pragma once

#include <any>
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "utils/utils.hpp"
#include "utils/resource.hpp"
#include "render/mesh.hpp"
#include "render/shader.hpp"
#include "render/texture.hpp"

class Model: public Res<Model> {
public:
	static ptr create(const std::string& name);
	virtual ~Model();

	inline void setMatrix(const glm::mat4& model) { _mat = model; }
	void draw(const glm::mat4& view, const glm::mat4& proj);

protected:
	bool initShader();
	bool initGL();

public:
	Attributes attrs;
private:
	Config _conf;
	Mesh::ptr _mesh;
	Shader::ptr _shader;
	GLuint _vao{0};
	GLint _lpos{-1}, _lcolor{-1}, _luv{-1}, _lnormal{-1}, _ltex{-1};
	glm::mat4 _mat{1.0f};
};

using ModelMgr = ResMgr<std::string, Model>;
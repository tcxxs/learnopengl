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
#include "render/camera.hpp"
#include "render/light.hpp"

class ModelProto;
class ModelInst : public ResInst<ModelProto, ModelInst> {
public:
	static ptr create(const proto_ptr& proto, const Config::node& conf);

	inline void setMatrix(const glm::mat4& model) { _mat = model; }
	void draw(const Shader::ptr& shader);

public:
	Attributes attrs;
private:
	Config _conf;
	glm::mat4 _mat{1.0f};
};

class ModelProto : public ResProto<ModelProto, ModelInst> {
public:
	static ptr create(const std::string& name);
	virtual ~ModelProto(); 

	void draw(const Camera::ptr& cam, const std::map<std::string, LightProto::ptr>& lights);

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
	GLint _lpos{-1}, _lcolor{-1}, _luv{-1}, _lnormal{-1};
};

using ModelProtoMgr = ResMgr<ModelProto>;

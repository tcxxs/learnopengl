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

class ModelProto;
class ModelInst : public Res<ModelInst> {
public:
	static ptr create(const std::shared_ptr<ModelProto>& proto, const Config::node& conf);

	inline void setMatrix(const glm::mat4& model) { _mat = model; }
	void draw(const Shader::ptr& shader);

public:
	Attributes attrs;

private:
	Config _conf;
	std::shared_ptr<ModelProto> _proto;
	glm::mat4 _mat{1.0f};
};

class ModelProto : public Res<ModelProto> {
public:
	static ptr create(const std::string& name);
	virtual ~ModelProto(); 

	template <typename ...ARGS>
	inline ModelInst::ptr instance(ARGS... args) {
		ModelInst::ptr inst = ModelInst::create(shared_from_this(), args...);
		if (!inst)
			return {};

		_insts[inst->getID()] = inst;
		return inst;
	}

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
	GLint _lpos{-1}, _lcolor{-1}, _luv{-1}, _lnormal{-1};
	std::map<ModelInst::idt, ModelInst::ptr> _insts;
};

using ModelProtoMgr = ResMgr<ModelProto>;

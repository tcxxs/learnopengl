#pragma once

#include <any>
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
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
	bool _loadAssimp();
	bool _loadNode(const std::filesystem::path& path, aiNode *node, const aiScene *scene);
	bool _initShader();

public:
	Attributes attrs;
private:
	static Assimp::Importer _imp;
	Config _conf;
	std::vector<Mesh::ptr> _meshs;
	Shader::ptr _shader;
};

using ModelProtoMgr = ResMgr<ModelProto>;

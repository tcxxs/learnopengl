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

	inline const glm::mat4& getMatrix() const { return _mat; }
	inline void setMatrix(const glm::mat4& model) { _mat = model; }

public:
	Attributes attrs;
private:
	glm::mat4 _mat{1.0f};
};

class ModelProto : public ResProto<ModelProto, ModelInst> {
public:
	static ptr create(const std::string& name);

	void draw(const Camera::ptr& cam, const std::map<std::string, LightProto::ptr>& lights);

protected:
	bool _loadAssimp();
	bool _loadNode(aiNode *node, const aiScene *scene);

private:
	inline static Assimp::Importer _imp;
	Config _conf;
	std::vector<Mesh::ptr> _meshs;
};

using ModelProtoMgr = ResMgr<ModelProto>;

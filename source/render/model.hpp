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
#include "render/command.hpp"

class ModelProto;
class ModelInst: public ResInst<ModelProto, ModelInst> {
public:
	static ptr create(const proto_ptr& proto, const Config::node& conf);

	void draw(CommandQueue& cmds);

	inline const glm::mat4& getMatrix() const { return _mat; }
	inline void setMatrix(const glm::mat4& model) { _mat = model; }

	inline bool changeShader(const std::string& name) {
	}

public:
	Attributes attrs;

private:
	glm::mat4 _mat{1.0f};
	std::vector<MeshInst::ptr> _meshs;
};

class ModelProto: public ResProto<ModelProto, ModelInst> {
public:
	using meshvec = std::vector<MeshProto::ptr>;

	static ptr create(const std::string& name);

	inline const meshvec& getMeshs() const { return _meshs; }

protected:
	bool _loadAssimp();
	bool _loadNode(aiNode* node, const aiScene* scene);

private:
	inline static Assimp::Importer _imp;
	Config _conf;
	meshvec _meshs;
};

using ModelProtoMgr = ResMgr<ModelProto>;

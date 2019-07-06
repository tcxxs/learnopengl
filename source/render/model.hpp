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
	virtual ~ModelInst();

	int draw(CommandQueue& cmds);

	inline bool changeShader(const std::string& name) {}
private:
	void _addInstance(const Config::node& conf);
	bool _initInstance();

public:
	Attributes attrs;

private:
	GLuint _vbo{0};
	std::vector<glm::mat4> _mats;
	std::vector<MeshInst::ptr> _meshs;
};

class ModelProto: public ResProto<ModelProto, ModelInst> {
public:
	using meshvec = std::vector<MeshProto::ptr>;

	static ptr create(const std::string& name);

	inline const meshvec& getMeshs() const { return _meshs; }

protected:
	bool _loadAssimp(const Config::node& conf);
	bool _loadNode(const Config::node& conf, aiNode* node, const aiScene* scene);
	bool _loadVertex(const Config::node& conf);

private:
	inline static Config _confs;
	inline static Assimp::Importer _imp;
	meshvec _meshs;
};

using ModelProtoMgr = ResMgr<ModelProto>;

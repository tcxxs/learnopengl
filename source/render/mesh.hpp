#pragma once

#include "glad/glad.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "utils/pattern.hpp"
#include "utils/utils.hpp"
#include "utils/resource.hpp"
#include "render/shader.hpp"
#include "render/camera.hpp"
#include "render/light.hpp"

struct Vertex {
	glm::vec3 pos;
	glm::vec2 uv;
	glm::vec3 normal;
};

class MeshProto;
class MeshInst: public ResInst<MeshProto, MeshInst> {
public:
	static ptr create(const proto_ptr& proto, const std::string& shader);

	inline void draw(const Camera::ptr& cam,
	                 const std::map<std::string, LightProto::ptr>& lights,
	                 const glm::mat4& model,
	                 const Attributes& mattrs);

private:
	ShaderInst::ptr _shader;
};

class MeshProto: public ResProto<MeshProto, MeshInst> {
public:
	static ptr create(const Config& conf, const aiMesh* ms, const aiScene* scene);
	virtual ~MeshProto();

	inline const GLuint getVAO() const { return _vao; }
	inline const GLuint getVBO() const { return _vbo; }
	inline const GLuint getIBO() const { return _ibo; }

	inline const ShaderInst::ptr& getShader(const std::string& name) const {
		const auto& it = _shaders.find(name);
		if (it == _shaders.end())
			return ShaderInst::empty;
		else
			return it->second;
	}
	inline const ShaderInst::ptr& getShaderDefault() const {
		return _shaders.begin()->second;
	}

	void draw(const Camera::ptr& cam,
	          const std::map<std::string, LightProto::ptr>& lights,
	          const glm::mat4& model,
	          const Attributes& mattrs,
	          const ShaderInst::ptr& shader);

protected:
	bool _loadVertex(const aiMesh* mesh);
	bool _loadMaterial(const std::filesystem::path& path, const aiMesh* mesh, const aiScene* scene);
	bool _loadTexture(const std::filesystem::path& path, const aiMaterial* mat, const aiTextureType type, const std::string& name);
	bool _initGL();
	bool _initShader(const Config::node& conf);

public:
	Attributes attrs;

private:
	std::vector<Vertex> _verts;
	std::vector<GLuint> _inds;
	GLuint _vao{0}, _vbo{0}, _ibo{0};
	std::map<std::string, ShaderInst::ptr> _shaders;
};

inline void MeshInst::draw(const Camera::ptr& cam,
                           const std::map<std::string, LightProto::ptr>& lights,
                           const glm::mat4& model,
                           const Attributes& mattrs) {
	_proto->draw(cam, lights, model, mattrs, _shader);
}

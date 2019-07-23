#pragma once

#include <typeindex>
#include <tuple>
#include "glad/glad.h"
#include "config.hpp"
#include "utils/resource.hpp"
#include "utils/utils.hpp"

class VertexProto;
class VertexInst: public ResInst<VertexProto, VertexInst> {
public:
	static ptr create(const proto_ptr& proto);

	bool setAttr(const std::string& name, GLint loc, GLint type);
	bool bindBuffer(GLuint vao, GLuint vbo);

private:
	std::map<std::string, GLint> _attrs;
};

class VertexProto: public ResProto<VertexProto, VertexInst> {
public:
	using attrtype = std::tuple<size_t, GLenum, GLenum, int, int, int>;
	using attrinfo = std::tuple<attrtype, int>;

	static ptr create(const std::string& name);

	inline const int getDivisor() const { return _divisor; }
	inline const attrinfo& findInfo(const std::string& name) const {
		const auto& it = _attrs.find(name);
		if (it == _attrs.end())
			return _typeempty;
		else
			return it->second;
	}
	inline const GLuint getBind() const { return _bind; }
	inline const GLuint getSize() const { return _size; }

private:
	inline static Config _confs;
	inline static GLuint _binding{0};
	inline static std::map<std::string, GLint> _typenames{
	    {"int", GL_INT},
	    {"float", GL_FLOAT},
	    {"vec2", GL_FLOAT_VEC2},
	    {"vec3", GL_FLOAT_VEC3},
	    {"mat4", GL_FLOAT_MAT4},
	};
	inline static std::map<GLenum, attrtype> _typeinfos{
		// 类型： C++类型，GL类型，元素类型，元素大小，x行，y列
	    {GL_INT, {typeid(int).hash_code(), GL_INT, GL_INT, (int)sizeof(int), 1, 1}},
	    {GL_FLOAT, {typeid(float).hash_code(), GL_FLOAT, GL_FLOAT, (int)sizeof(float), 1, 1}},
	    {GL_FLOAT_VEC2, {typeid(glm::vec2).hash_code(), GL_FLOAT_VEC2, GL_FLOAT, (int)sizeof(float), 1, 2}},
	    {GL_FLOAT_VEC3, {typeid(glm::vec3).hash_code(), GL_FLOAT_VEC3, GL_FLOAT, (int)sizeof(float), 1, 3}},
	    {GL_FLOAT_MAT4, {typeid(glm::mat4).hash_code(), GL_FLOAT_MAT4, GL_FLOAT, (int)sizeof(float), 4, 4}},
	};
	inline static attrinfo _typeempty{{0, 0, 0, 0, 0, 0}, 0};

	int _divisor{0};
	std::map<std::string, attrinfo> _attrs;
	int _size{0};
	GLuint _bind{0};
};

using VertexProtoMgr = ResMgr<VertexProto>;

struct VertexBase {
	glm::vec3 pos;
	glm::vec2 uv;
	glm::vec3 normal;
	glm::vec3 tangent;
};

struct VertexInstance {
	GLfloat mat[4][4];
};
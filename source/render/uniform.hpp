#pragma once

#include <typeindex>
#include "glad/glad.h"
#include "config.hpp"
#include "utils/resource.hpp"

class UniformProto;
class UniformInst: public ResInst<UniformProto, UniformInst> {
public:
	static ptr create(const proto_ptr& proto);
	virtual ~UniformInst();

	void bind(const GLuint point);
	template <typename V>
	bool setVar(const std::string& key, const V& var);
	template <typename V>
	inline bool setVar(int offset, const V& var) { return setVar(offset, &var, (int)sizeof(var)); }
	template <>
	inline bool setVar(int offset, const glm::vec3& var) { return setVar(offset, glm::value_ptr(var), (int)sizeof(var)); }
	template <>
	inline bool setVar(int offset, const glm::mat4& var) { return setVar(offset, glm::value_ptr(var), (int)sizeof(var)); }
	bool setVar(int offset, const void* addr, const int size);

private:
	GLuint _ubo{0};
};

class UniformProto: public ResProto<UniformProto, UniformInst> {
public:
	using typeinfo = std::tuple<size_t>;
	using varinfo = std::tuple<const typeinfo, int>;
	using glinfo = std::map<std::string, GLint>;
	static ptr create(const std::string& name, const GLint size, const glinfo& vars);

	inline int getSize() const { return _size; }
	inline const varinfo& getVar(const std::string& key) const {
		const auto& find = _vars.find(key);
		if (find == _vars.end())
			return _varempty;
		else
			return find->second;
	}

private:
	inline static Config _confs;
	inline static varinfo _varempty{{0}, 0};
	inline static std::map<std::string, typeinfo> _types{
	    {"int", {typeid(int).hash_code()}},
	    {"float", {typeid(float).hash_code()}},
	    {"vec3", {typeid(glm::vec3).hash_code()}},
	    {"mat4", {typeid(glm::mat4).hash_code()}},
	};

	std::map<std::string, varinfo> _vars;
	int _size{0};
};

using UniformProtoMgr = ResMgr<UniformProto>;

template <typename V>
bool UniformInst::setVar(const std::string& key, const V& var) {
	const UniformProto::varinfo& info = prototype()->getVar(key);
	const UniformProto::typeinfo& type = std::get<0>(info);
	if (typeid(V).hash_code() != std::get<0>(type)) {
		std::cout << "uniform: " << prototype()->getName() << ", type error: " << key << std::endl;
		return false;
	}

	return setVar(std::get<1>(info), var);
}

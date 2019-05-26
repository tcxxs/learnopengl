#pragma once

#include <iostream>
#include <any>
#include <map>
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "utils/resource.hpp"

void oglFeature();
bool oglError();

template <typename... Args>
inline std::string string_format(const std::string& format, Args... args) {
	size_t size = (size_t)std::snprintf(nullptr, 0, format.c_str(), args...) + (size_t)1;
	std::unique_ptr<char[]> buf(new char[size]);
	std::snprintf(buf.get(), size, format.c_str(), args...);
	return std::string(buf.get(), buf.get() + size - 1);
}

class Attributes {
public:
	using AttrMap = std::map<std::string, std::any>;

	inline AttrMap::iterator begin() { return _attrs.begin(); }
	inline AttrMap::iterator end() { return _attrs.end(); }

	inline AttrMap::const_iterator begin() const { return _attrs.begin(); }
	inline AttrMap::const_iterator end() const { return _attrs.end(); }

	void setAttr(const std::string& key, const std::any& value) {
		_attrs[key] = value;
	}
	template <typename V>
	const V& getAttr(const std::string& key) {
		return std::any_cast<V&>(_attrs[key]);
	}
	template <>
	const std::any& getAttr(const std::string& key) {
		return _attrs[key];
	}
	template <typename V>
	const V& getAttr(const std::string& key, const V& dft) {
		const auto& it = _attrs.find(key);
		if (it == _attrs.end())
			return dft;
		else
			return std::any_cast<V&>(*it);
	}

	inline void updateAttrs(const Attributes& attrs) {
		for (const auto& it: attrs) {
			_attrs[it.first] = it.second;
		}
	}
	bool updateConf(const Config::node& doc);

private:
	AttrMap _attrs;
};
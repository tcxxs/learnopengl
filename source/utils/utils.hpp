#pragma once

#include <iostream>
#include <any>
#include <map>
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"

void oglFeature();
bool oglError();

class Attributes {
public:
	void setAttr(const std::string& key, const std::any& value) {
		_attrs[key] = value;
	}
	template <typename V>
	const V& getAttr(const std::string& key) {
		return std::any_cast<V>(_attrs[key]);
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
			return std::any_cast<V>(*it);
	}

private:
	std::map<std::string, std::any> _attrs;
};
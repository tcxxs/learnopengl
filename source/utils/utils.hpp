#pragma once

#include <iostream>
#include <any>
#include <map>
#include <functional>
#include <filesystem>
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "utils/resource.hpp"

bool oglError();

template <typename... Args>
inline std::string string_format(const std::string& format, Args... args) {
	size_t size = (size_t)std::snprintf(nullptr, 0, format.c_str(), args...) + (size_t)1;
	std::unique_ptr<char[]> buf(new char[size]);
	std::snprintf(buf.get(), size, format.c_str(), args...);
	return std::string(buf.get(), buf.get() + size - 1);
}

using strcube = std::array<std::string, 6>;
namespace YAML {
template <>
struct convert<glm::vec2> {
	static Node encode(const glm::vec2& rhs) {
		Node node;
		node.push_back(rhs.x);
		node.push_back(rhs.y);
		return node;
	}

	static bool decode(const Node& node, glm::vec2& rhs) {
		if (!node.IsSequence() || node.size() != 2) {
			return false;
		}

		rhs.x = node[0].as<float>();
		rhs.y = node[1].as<float>();
		return true;
	}
};

template <>
struct convert<glm::vec3> {
	static Node encode(const glm::vec3& rhs) {
		Node node;
		node.push_back(rhs.x);
		node.push_back(rhs.y);
		node.push_back(rhs.z);
		return node;
	}

	static bool decode(const Node& node, glm::vec3& rhs) {
		if (!node.IsSequence() || node.size() != 3) {
			return false;
		}

		rhs.x = node[0].as<float>();
		rhs.y = node[1].as<float>();
		rhs.z = node[2].as<float>();
		return true;
	}
};

template <>
struct convert<strcube> {
	static Node encode(const strcube& rhs) {
		Node node;
		for (const auto& it: rhs)
			node.push_back(it);
		return node;
	}

	static bool decode(const Node& node, strcube& rhs) {
		if (!node.IsSequence() || node.size() != 6) {
			return false;
		}

		for (int i = 0; i < 6; ++i)
			rhs[i] = node[i].as<std::string>();
		return true;
	}
};
} // namespace YAML

class Config {
public:
	using node = YAML::Node;
	using genfunc = std::function<std::any(const Config::node&)>;

	static const node visit(const node& doc, const std::string& path);
	static std::any guess(const node& doc);
	inline static bool valid(const node& doc) { return doc.IsDefined() && !doc.IsNull(); }

	bool load(const std::filesystem::path& path);
	const node& root() const {
		return _doc;
	}
	inline const node operator[](const std::string& path) const {
		return visit(_doc, path);
	}

public:
	inline static node empty{};
	inline static genfunc gen;

private:
	node _doc;
};

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
	bool hasAttr(const std::string& key) const { return _attrs.find(key) != _attrs.end(); }
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

class Logger {
public:
	inline static void init(const Config::node& conf) {
		std::filesystem::path path = std::filesystem::current_path() / conf[0].as<std::string>();
		_file = fopen(path.string().c_str(), "wb");

		for (const auto& type: conf[1]) {
			_filter.insert(type.as<std::string>());
		}
	}

	template <typename... Args>
	inline static void out(const std::string& type, const std::string& format, Args... args) {
		if (_filter.count(type) <= 0)
			return;

		std::string fmt = string_format("[%s] %s\n", type.c_str(), format.c_str());
		std::string log = string_format(fmt, args...);
		_logs.push_back(log);
		std::fputs(log.c_str(), _file);
		std::fflush(_file);
	}

	inline static const std::vector<std::string>& get() { return _logs; }

private:
	inline static std::FILE* _file{nullptr};
	inline static std::set<std::string> _filter;
	inline static std::vector<std::string> _logs;
};

#define LOG(type, format, ...) Logger::out(type, format, ##__VA_ARGS__)
#define INFO(format, ...) Logger::out("info", format, ##__VA_ARGS__)
#define ERR(format, ...) Logger::out("error", format, ##__VA_ARGS__)
#define DEBUG(format, ...) Logger::out("debug", format, ##__VA_ARGS__)

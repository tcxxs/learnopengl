#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "yaml-cpp/yaml.h"
#include "utils/pattern.hpp"

bool readFile(const std::filesystem::path& path, std::string& content);

template <typename R>
class Res : public std::enable_shared_from_this<R> {
public:
	using ptr = std::shared_ptr<R>;
	virtual ~Res() = default;

protected:
	Res() = default;
};

template <typename K, typename R>
class ResContainer : public NoCopy {
public:
	using resptr = typename R::ptr;
	using conmap = std::map<K, resptr>;

	inline void add(const K& key, const resptr& res) {
		_con[key] = res;
	}

	inline void del(const K& key) {
		_con.erase(key);
	}

	inline const resptr& get(const K& key) {
		auto it = _con.find(key);
		if (it == _con.end())
			return _empty;
		else
			return it->second;
	}

	inline const resptr& req(const K& key) {
		auto it = _con.find(key);
		if (it == _con.end()) {
			resptr r = R::create(key);
			if (!r)
				return _empty;

			auto rit = _con.insert(conmap::value_type(key, r));
			return rit.first->second;
		}
		else
			return it->second;
	}

	inline conmap container() {
		return _con;
	}

private:
	inline static const resptr _empty{};
	conmap _con;
};

template <typename K, typename R>
using ResMgr = Singleton<ResContainer<K, R>>;

namespace YAML {
template<>
struct convert<glm::vec3> {
  static Node encode(const glm::vec3& rhs) {
    Node node;
    node.push_back(rhs.x);
    node.push_back(rhs.y);
    node.push_back(rhs.z);
    return node;
  }

  static bool decode(const Node& node, glm::vec3& rhs) {
    if(!node.IsSequence() || node.size() != 3) {
      return false;
    }

    rhs.x = node[0].as<float>();
    rhs.y = node[1].as<float>();
    rhs.z = node[2].as<float>();
    return true;
  }
};
}

class Config {
public:
	using node = YAML::Node;
	static const node visit(const node& doc, const std::string& path);

	bool load(const std::filesystem::path& path);
	inline const node operator[] (const std::string& path) const {
		return visit(_doc, path);
	}

private:
	inline static node _empty{};
	node _doc;
};

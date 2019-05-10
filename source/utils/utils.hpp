#pragma once

#include <iostream>
#include <filesystem>
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "yaml-cpp/yaml.h"

void oglFeature();
bool oglError();

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

	bool load(const std::filesystem::path& path);
	const node operator[] (const std::string& path) const;

private:
	inline static node _empty{};
	node _doc;
};


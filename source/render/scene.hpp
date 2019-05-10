#pragma once

#include "glad/glad.h"
#include "utils/utils.hpp"
#include "utils/resource.hpp"
#include "render/model.hpp"

class Scene: public Res<Scene> {
public:
	static ptr create(const std::string& name);

	void addModel(const Config::node& conf);

private:
	Config _conf;
	std::map<std::string, Model::ptr> _models;
};

using SceneMgr = ResMgr<std::string, Scene>;
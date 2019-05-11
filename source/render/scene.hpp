#pragma once

#include "glad/glad.h"
#include "utils/utils.hpp"
#include "utils/resource.hpp"
#include "render/camera.hpp"
#include "render/model.hpp"
#include "render/light.hpp"

class Scene: public Res<Scene> {
public:
	static ptr create(const std::string& name);
	virtual ~Scene();

	void addCamera(const Config::node& conf);
	void addModel(const Config::node& conf);
	void addLight(const Config::node& conf);

	inline void active() {
		Scene::current = shared_from_this();
	}
	void draw();

public:
	inline static ptr current{};
private:
	Config _conf;
	Camera::ptr _cam;
	std::map<std::string, Model::ptr> _models;
	std::map<std::string, Light::ptr> _lights;
};

using SceneMgr = ResMgr<std::string, Scene>;
#pragma once

#include "glad/glad.h"
#include "utils/utils.hpp"
#include "utils/resource.hpp"
#include "render/camera.hpp"
#include "render/model.hpp"
#include "render/light.hpp"
#include "render/frame.hpp"

class Scene: public Res<Scene> {
public:
	static ptr create(const std::string& name);
	virtual ~Scene();

	const Camera::ptr& getCamera() const { return _cam; }
	void addCamera(const Config::node& conf);
	void addLight(const Config::node& conf);
	void addModel(const Config::node& conf);
	void addPass(const Config::node& conf);

	inline void active() {
		Scene::current = shared_from_this();
	}
	void draw();
	void drawCommand(const Command& cmd);

public:
	inline static ptr current{};

private:
	Camera::ptr _cam;
	std::vector<LightInst::ptr> _lights;
	std::vector<ModelInst::ptr> _models;
	std::map<std::string, Frame::ptr> _frames;
};

using SceneMgr = ResMgr<Scene>;
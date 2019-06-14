#pragma once

#include "glad/glad.h"
#include "utils/utils.hpp"
#include "utils/resource.hpp"
#include "render/camera.hpp"
#include "render/model.hpp"
#include "render/light.hpp"
#include "render/frame.hpp"
#include "render/shader.hpp"
#include "render/post.hpp"

struct Pass 
{
	std::string name;
	Post::ptr post;
	std::vector<Frame::ptr> ins;
	Frame::ptr out;
	std::set<std::string> states;
};

class Scene: public Res<Scene> {
public:
	static ptr create(const std::string& name);
	virtual ~Scene();

	const Camera::ptr& getCamera() const { return _cam; }
	void addCamera(const Config::node& conf);
	bool addLight(const Config::node& conf);
	bool addModel(const Config::node& conf);
	bool addPass(const Config::node& conf);

	inline void active() {
		Scene::current = shared_from_this();
	}
	void draw();
	void drawScene(const Pass& pass);
	void drawCommand(const Command& cmd);
	void drawPost(const Pass& pass);

public:
	inline static ptr current{};

private:
	Camera::ptr _cam;
	std::vector<LightInst::ptr> _lights;
	std::vector<ModelInst::ptr> _models;
	std::vector<Pass> _pass;
	std::map<std::string, Frame::ptr> _frames;
};

using SceneMgr = ResMgr<Scene>;
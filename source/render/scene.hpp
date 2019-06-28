#pragma once

#include <functional>
#include "glad/glad.h"
#include "utils/utils.hpp"
#include "utils/resource.hpp"
#include "render/camera.hpp"
#include "render/model.hpp"
#include "render/light.hpp"
#include "render/frame.hpp"
#include "render/shader.hpp"
#include "render/post.hpp"
#include "render/uniform.hpp"

struct Pass 
{
	using statefunc = std::function<void()>;

	std::string name;
	std::set<Shader::ptr> shaders;
	Post::ptr post;
	std::vector<Frame::ptr> ins;
	Frame::ptr out;
	std::vector<statefunc> states;
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
	bool addUniform(const std::string& name, int count = 1);

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
	std::map<std::string, UniformInst::ptr> _uniforms;
};

using SceneMgr = ResMgr<Scene>;
#pragma once

#include <functional>
#include "glad/glad.h"
#include "utils/utils.hpp"
#include "utils/resource.hpp"
#include "render/camera.hpp"
#include "render/model.hpp"
#include "render/light.hpp"
#include "render/frame.hpp"
#include "render/uniform.hpp"
#include "render/pass.hpp"

// TODO: 是不是该有个render context
// TODO: 写插件从blender导出是正路
class Scene: public Res<Scene> {
public:
	using genfunc = std::function<std::any(const Config::node&)>;
	static ptr create(const std::string& name);
	virtual ~Scene();

	const Camera::ptr& getCamera() const { return _cam; }
	void addCamera(const std::string& name, const Config::node& conf);
	bool addLight(const Config::node& conf);
	bool addModel(const Config::node& conf);
	bool addPass(const Config::node& conf);

	inline void active() {
		Scene::current = shared_from_this();
	}
	void draw();
	void drawUniforms(const Pass::ptr& pass);
	void drawCommand(const Command& cmd);

	std::any generateConf(const Config::node& conf);

private:
	bool _initUniform(const std::string& name, int count = 1);
	bool _initFrame(const Config::node& conf);

	std::any _genCamera(const Config::node& conf);
	std::any _genLight(const Config::node& conf);
	std::any _genFrame(const Config::node& conf);

public:
	inline static ptr current{};

private:
	std::map<std::string, genfunc> _cfuncs;
	Camera::ptr _cam;
	std::map<std::string, Camera::ptr> _cams;
	std::vector<LightInst::ptr> _lights;
	std::vector<ModelInst::ptr> _models;
	std::vector<Pass::ptr> _pass;
	std::map<std::string, Frame::ptr> _frames;
	std::map<std::string, UniformInst::ptr> _uniforms;
};

using SceneMgr = ResMgr<Scene>;
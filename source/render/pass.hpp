#pragma once

#include <functional>
#include "glad/glad.h"
#include "config.hpp"
#include "utils/resource.hpp"
#include "render/frame.hpp"
#include "render/shader.hpp"
#include "render/post.hpp"
#include "render/command.hpp"
#include "render/model.hpp"
#include "render/uniform.hpp"
#include "render/camera.hpp"

struct ShaderAttrs {
	using frameattach = std::pair<Frame::ptr, std::string>;
	std::map<std::string, frameattach> frames;
	Attributes attrs;
};

// TODO: 定义pass生成器，自动生成类似shadow map之类的pass
// 只要pass自身功能完备，生成器只是帮助scene.yml少写配置
class Pass: public Res<Pass> {
public:
	using genfunc = std::function<std::any(const Config::node&)>;
	using statefunc = std::function<void()>;
	using runfunc = std::function<bool()>;
	using modelvec = std::vector<ModelInst::ptr>;
	static ptr create(const Config::node& conf);

	inline bool getRun() const {
		if (_run)
			return _run();
		else
			return true;
	}
	inline std::pair<int, int> getView() const {
		if (_outframe)
			return _outframe->getView(_outcolors);
		else
			return {SystemMgr::inst().getWidth(), SystemMgr::inst().getHeight()};
	}
	inline const Camera::ptr& getCamera() const { return _cam; }
	inline const std::set<Shader::ptr>& getShaders() const { return _shaders; }

	void drawBegin();
	void drawEnd();
	int drawPass(CommandQueue& cmds, const modelvec& models);

private:
	bool _initConf(const Config::node& conf);
	bool _initRun(const Config::node& conf);
	bool _initState(const Config::node& conf);
	bool _initShaderAttrs(const Config::node& conf, const Shader::ptr& shader);
	bool _initShader(const Config::node& conf);
	bool _initPost(const Config::node& conf);
	bool _initOutput(const Config::node& conf);

	void _stateClear(const Config::node& conf);
	void _stateDepth(const Config::node& conf);
	void _stateCull(const Config::node& conf);

private:
	runfunc _run;
	bool _run_once{true};
	Camera::ptr _cam;
	std::vector<statefunc> _states;
	std::set<Shader::ptr> _shaders;
	std::set<Post::ptr> _posts;
	std::map<std::string, ShaderAttrs> _sattrs;
	Frame::ptr _outframe;
	std::map<GLuint, std::string> _outcolors;
};

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
	using modelvec = std::vector<ModelInst::ptr>;
	static ptr create(const Config::node& conf);

	inline std::pair<int, int> getView() const {
		if (_outframe)
			return _outframe->getView();
		else
			return {EventMgr::inst().getWidth(), EventMgr::inst().getHeight()};
	}
	inline const Camera::ptr& getCamera() const { return _cam; }
	inline const std::set<Shader::ptr>& getShaders() const { return _shaders; }

	void drawBegin();
	inline void drawEnd() { 
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// TODO: 可以只dirty用到的color
		if (_outframe)
			_outframe->setDirtyAll();
	}
	int drawPass(CommandQueue& cmds, const modelvec& models);

private:
	bool _initConf(const Config::node& conf);
	bool _initState(const Config::node& conf);
	bool _initShaderAttrs(const Config::node& conf, const Shader::ptr& shader);
	bool _initShader(const Config::node& conf);
	bool _initPost(const Config::node& conf);
	bool _initOutput(const Config::node& conf);

	void _stateClear(const Config::node& conf);
	void _stateDepth(const Config::node& conf);
	void _stateFace(const Config::node& conf);

private:
	Camera::ptr _cam;
	std::vector<statefunc> _states;
	std::set<Shader::ptr> _shaders;
	std::set<Post::ptr> _posts;
	std::map<std::string, ShaderAttrs> _sattrs;
	Frame::ptr _outframe;
	std::map<GLuint, std::string> _outcolors;
};

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
	std::map<std::string, Frame::ptr> ins;
	std::map<std::string, Frame::ptr> outs;
	Attributes attrs;
};

// TODO: 定义pass生成器，自动生成类似shadow map之类的pass
// 只要pass自身功能完备，生成器只是帮助scene.yml少写配置
class Pass: public Res<Pass> {
public:
	using genfunc = std::function<std::any(const Config::node&)>;
	using frameinfo = std::pair<std::string, Frame::ptr>;

	using statefunc = std::function<void()>;
	using framemap = std::map<std::string, Frame::ptr>;
	using modelvec = std::vector<ModelInst::ptr>;
	static ptr create(const Config::node& conf, const genfunc& gen);

	inline const std::set<Shader::ptr>& getShaders() const { return _shaders; }
	inline const Camera::ptr& getCamera() const { return _cam; }

	void drawBegin();
	inline void drawEnd() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
	int drawPass(CommandQueue& cmds, const modelvec& models);

private:
	bool _initConf(const Config::node& conf, const genfunc& gen);
	bool _initShaderAttrs(const Config::node& conf, const genfunc& gen, const Shader::ptr& shader);
	bool _initShader(const Config::node& conf, const genfunc& gen);
	bool _initPost(const Config::node& conf, const genfunc& gen);
	bool _initState(const Config::node& conf);

	void _stateClear(const Config::node& conf);
	void _stateDepth(const Config::node& conf);
	void _stateFace(const Config::node& conf);

private:
	Camera::ptr _cam;
	std::set<Shader::ptr> _shaders;
	std::set<Post::ptr> _posts;
	std::map<std::string, ShaderAttrs> _sattrs;
	Frame::ptr _out;
	std::vector<statefunc> _states;
};

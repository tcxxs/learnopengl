#pragma once

#include <functional>
#include "glad/glad.h"
#include "config.hpp"
#include "utils/resource.hpp"
#include "render/frame.hpp"
#include "render/shader.hpp"
#include "render/post.hpp"

class Pass: public Res<Pass> {
public:
	using statefunc = std::function<void()>;
	using framemap = std::map<std::string, Frame::ptr>;
	static ptr create(const Config::node& conf, framemap& frames);

	inline const std::set<Shader::ptr>& getShaders() const { return _shaders; }
	inline const std::string& getCamera() const { return _cam; }

	void drawBegin();
	void drawEnd();
	inline bool drawPost() {
		if (_post) {
			_post->draw(_ins);
			return true;
		}
		else
			return false;
	}

private:
	bool _initConf(const Config::node& conf);
	bool _initFrame(const Config::node& conf, framemap& frames);
	bool _initState(const Config::node& conf);

	void _stateClear(const Config::node& conf);
	void _stateDepth(const Config::node& conf);

private:
	std::string _cam;
	std::set<Shader::ptr> _shaders;
	Post::ptr _post;
	std::vector<Frame::ptr> _ins;
	Frame::ptr _out;
	std::vector<statefunc> _states;
};

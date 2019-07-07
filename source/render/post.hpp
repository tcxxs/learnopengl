#pragma once

#include "glad/glad.h"
#include "config.hpp"
#include "utils/resource.hpp"
#include "utils/utils.hpp"
#include "render/shader.hpp"
#include "render/frame.hpp"

class Post: public Res<Post> {
public:
	using framevec = std::vector<Frame::ptr>;

	static ptr create(const std::string& name);
	virtual ~Post();

	void draw(const framevec& ins);
	inline const Shader::ptr& getShader() const { return _shader; }

private:
	bool static _initVBO();
	bool _initVAO();

public:
	Attributes attrs;
private:
	inline static Config _confs;
	inline static GLuint _vbo{0};
	Shader::ptr _shader;
	GLuint _vao{0};
	std::vector<std::string> _ins;
};

using PostMgr = ResMgr<Post>;

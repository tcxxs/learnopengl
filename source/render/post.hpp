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
	bool _initGL();

public:
	Attributes attrs;
private:
	inline static Config _confs;
	inline static float _vertexs[] = {POST_VERTEX};
	inline static GLuint _vao{0}, _vbo{0};
	Shader::ptr _shader;
	std::vector<std::string> _ins;
};

using PostMgr = ResMgr<Post>;

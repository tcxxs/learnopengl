#pragma once

#include "glad/glad.h"
#include "config.hpp"
#include "utils/resource.hpp"
#include "utils/utils.hpp"
#include "render/shader.hpp"
#include "render/frame.hpp"
#include "render/command.hpp"

class Post: public Res<Post> {
public:
	using framevec = std::vector<Frame::ptr>;

	static ptr create(const std::string& name);
	virtual ~Post();

	int draw(CommandQueue& cmds);
	inline const Material::ptr& getMaterial() const { return _material; }

private:
	bool static _initVBO();
	bool _initVAO();

public:
	Attributes attrs;
private:
	inline static Config _confs;
	inline static GLuint _vbo{0};
	Material::ptr _material;
	GLuint _vao{0};
};

using PostMgr = ResMgr<Post>;

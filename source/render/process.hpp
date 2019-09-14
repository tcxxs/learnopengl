#pragma once

#include "glad/glad.h"
#include "config.hpp"
#include "utils/resource.hpp"
#include "utils/utils.hpp"
#include "render/shader.hpp"
#include "render/frame.hpp"
#include "render/command.hpp"

class Process: public Res<Process> {
public:
	static ptr create(const std::string& name);
	virtual ~Process();

	int draw(CommandQueue& cmds);
	inline const Material::ptr& getMaterial() const { return _material; }

private:
	void static _initVBO();
	bool _initVAO();
	bool _initCube(const Config::node& conf);

public:
	Attributes attrs;

private:
	inline static Config _confs;
	inline static std::map<std::string, std::pair<int, GLuint>> _shapes;
	GLuint _vao{0};
	GLuint _vbo;
	int _vsize{0};
	Material::ptr _material;
};

using PostMgr = ResMgr<Process>;

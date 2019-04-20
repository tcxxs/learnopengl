#pragma once

#include "glad/glad.h"
#include "event.hpp"
#include "utils/resource.hpp"

class Shader: public Res<Shader> {
public:
	static ptr create(const std::string& name);

	virtual ~Shader();

	bool loadProgram();
	bool useProgram();

private:
	bool _loadShader(int type, GLuint& shader);

private:
	std::string _name;
	GLuint _prog{0};
};

using ShaderMgr = ResMgr<std::string, Shader>;
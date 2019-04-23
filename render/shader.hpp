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

	inline GLint getVar(const std::string& name) const {
		auto it = _vars.find(name);
		if (it == _vars.end())
			return -1;
		else
			return it->second;
	}

private:
	bool _loadShader(int type, GLuint& shader);

private:
	std::string _name;
	GLuint _prog{0};
	std::map<std::string, GLint> _vars;
};

using ShaderMgr = ResMgr<std::string, Shader>;
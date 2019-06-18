#pragma once

#include "glad/glad.h"
#include "config.hpp"
#include "utils/utils.hpp"
#include "utils/resource.hpp"
#include "render/texture.hpp"

class Shader: public Res<Shader> {
public:
	static ptr create(const std::string& name);
	virtual ~Shader();

	void use();

	inline const GLint getVar(const std::string& name) const {
		auto it = _vars.find(name);
		if (it == _vars.end())
			return -1;
		else
			return it->second;
	}

	template <typename V>
	inline void setVar(const std::string& name, const V& var) {
		const GLint loc = getVar(name);
		if (loc < 0)
			return;
		setVar((GLuint)loc, var);
	}
	void setVar(const GLuint& loc, const std::any& var);
	inline void setVar(const GLuint& loc, const GLuint& var) {
		glUniform1i(loc, var);
	}
	inline void setVar(const GLuint& loc, const int& var) {
		glUniform1i(loc, var);
	}
	inline void setVar(const GLuint& loc, const float& var) {
		glUniform1f(loc, var);
	}
	inline void setVar(const GLuint& loc, const glm::vec3& var) {
		glUniform3fv(loc, 1, glm::value_ptr(var));
	}
	inline void setVar(const GLuint& loc, const Texture::ptr& var) {
		glActiveTexture(GL_TEXTURE0 + _tex);
		glBindTexture(var->getType(), var->getTexture());
		glUniform1i(loc, _tex);
		_tex += 1;
	}
	inline void setVars(const Attributes& attrs) {
		for (const auto& it : attrs) {
			setVar(it.first, it.second);
		}
	}

private:
	bool _loadShader(int type, GLuint& shader);
	bool _loadProgram();

private:
	GLuint _prog{0};
	int _tex{0};
	std::map<std::string, GLuint> _vars;
};

using ShaderMgr = ResMgr<Shader>;

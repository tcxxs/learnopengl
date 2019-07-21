#pragma once

#include "glad/glad.h"
#include "config.hpp"
#include "utils/utils.hpp"
#include "utils/resource.hpp"
#include "render/texture.hpp"
#include "render/vertex.hpp"

// TODO: 使用宏开关，缩减glsl代码量
class Shader: public Res<Shader> {
public:
	static ptr create(const std::string& name);
	virtual ~Shader();

	void use();

	inline const GLint getVar(const std::string& name) const {
		const auto& find = _vars.find(name);
		if (find == _vars.end())
			return -1;
		else
			return find->second;
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
	inline void setVar(const GLuint& loc, const glm::mat4& var) {
		glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(var));
	}
	inline void setVar(const GLuint& loc, const std::vector<glm::mat4>& var) {
		glUniformMatrix4fv(loc, (GLsizei)var.size(), GL_FALSE, glm::value_ptr(var[0]));
	}
	inline void setVar(const GLuint& loc, const Texture::val& var) {
		const auto& find = _samplers.find(loc);
		if (find == _samplers.end()) {
			std::printf("shader set var, texture not found , locale %d", loc);
			return;
		}
		glBindTextureUnit(find->second.first, var.tex);
	}
	inline void setVars(const Attributes& attrs) {
		for (const auto& it: attrs) {
			setVar(it.first, it.second);
		}
	}

	inline const VertexInst::ptr& getVert(const std::string& name) const {
		const auto& it = _verts.find(name);
		if (it == _verts.end())
			return VertexInst::empty;
		else
			return it->second;
	}
	bool bindVertex(const std::string& name, const GLuint vao, const GLuint vbo);

private:
	bool _loadShader(const std::string& ext, int type, GLuint& shader);
	bool _loadProgram();
	bool _loadVertex();
	bool _loadUniform();

private:
	GLuint _prog{0};
	std::map<std::string, GLuint> _vars;
	std::map<GLuint, std::pair<GLuint, GLenum>> _samplers;
	std::map<std::string, VertexInst::ptr> _verts;
};

using ShaderMgr = ResMgr<Shader>;

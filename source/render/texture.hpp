#pragma once

#include "glad/glad.h"
#include "utils/resource.hpp"
#include "utils/utils.hpp"

class Texture: public Res<Texture> {
public:
	struct val {
		val(GLuint t): tex(t){};
		val(GLenum p, GLuint t): type(p), tex(t){};
		GLenum type{GL_TEXTURE_2D};
		GLuint tex{0};
	};

	static ptr create(const std::string& name, const std::filesystem::path& path);
	inline static ptr create(const std::string& name) {
		std::filesystem::path path = std::filesystem::current_path() / "resource" / "texture" / name;
		return create(name, path);
	}
	static ptr create(const strcube& cube);
	virtual ~Texture();

	inline const val getValue() const { return {_type, _tex}; }

private:
	GLuint _tex{0};
	GLenum _type{0};
	int _w{0}, _h{0}, _n{0};
};

using TextureMgr = ResMgr<Texture>;
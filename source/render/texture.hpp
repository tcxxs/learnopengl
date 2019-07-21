#pragma once

#include "glad/glad.h"
#include "utils/resource.hpp"
#include "utils/utils.hpp"

class Texture: public Res<Texture> {
public:
	struct val {
		val(GLuint t, GLenum p = GL_TEXTURE_2D): tex(t), type(p){};
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

	static GLuint getDefault(const GLenum type);

	inline const val getValue() const { return {_tex, _type}; }

private:
	inline static std::map<GLenum, GLuint> _defaults;
	GLuint _tex{0};
	GLenum _type{0};
	int _w{0}, _h{0}, _n{0};
};

using TextureMgr = ResMgr<Texture>;
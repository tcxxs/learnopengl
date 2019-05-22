#pragma once

#include "glad/glad.h"
#include "utils/resource.hpp"

class Texture: public Res<Texture> {
public:
	static ptr create(const std::string& name, const std::filesystem::path& path);
	inline static ptr create(const std::string& name) {
		std::filesystem::path path = std::filesystem::current_path() / "resource" / "texture" / name;
		return create(name, path);
	}
	virtual ~Texture();

	inline const GLuint getTexture() const { return _tex; }

private:
	GLuint _tex{0};
	int _w{0}, _h{0}, _n{0};
};

using TextureMgr = ResMgr<Texture>;
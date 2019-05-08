#pragma once

#include "glad/glad.h"
#include "utils/resource.hpp"

class Texture: public Res<Texture> {
public:
	static ptr create(const std::string& name);
	virtual ~Texture();

	inline const GLuint getTexture() const { return _tex; }

private:
	GLuint _tex{0};
	int _w{0}, _h{0}, _n{0};
};

using TextureMgr = ResMgr<std::string, Texture>;
#include "texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

Texture::ptr Texture::create(const std::string& name, const std::filesystem::path& path) {
	Texture::ptr texture = std::make_shared<Texture>();
	texture->setName(name);

	texture->_type = GL_TEXTURE_2D;
	glGenTextures(1, &texture->_tex);
	glBindTexture(GL_TEXTURE_2D, texture->_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	bool hdr = false;
	const std::string ext = path.extension().string();
	if (ext == ".hdr")
		hdr = true;
	stbi_set_flip_vertically_on_load(true);
	if (hdr) {
		float* data = stbi_loadf(path.string().c_str(), &texture->_w, &texture->_h, &texture->_n, 3);
		if (!data) {
			std::cout << "stb image: " << path.string() << ", error: " << stbi_failure_reason() << std::endl;
			return {};
		}
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, texture->_w, texture->_h, 0, GL_RGB, GL_FLOAT, data);
		stbi_image_free(data);
	}
	else {
		unsigned char* data = stbi_load(path.string().c_str(), &texture->_w, &texture->_h, &texture->_n, 4);
		if (!data) {
			std::cout << "stb image: " << path.string() << ", error: " << stbi_failure_reason() << std::endl;
			return {};
		}
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->_w, texture->_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}
	
	//glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);	
	return texture;
}

Texture::ptr Texture::create(const strcube& cube) {
	Texture::ptr texture = std::make_shared<Texture>();

	glGenTextures(1, &texture->_tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture->_tex);

	std::filesystem::path path;
	int w, h, n;
	stbi_set_flip_vertically_on_load(false); // cubemap使用左手系
	for (int i = 0; i < 6; ++i) {
		path = std::filesystem::current_path() / "resource" / "texture" / cube[i];
		unsigned char* data = stbi_load(path.string().c_str(), &w, &h, &n, 4);
		if (!data) {
			std::cout << "stb image: " << path.string() << ", error: " << stbi_failure_reason() << std::endl;
			return {};
		}
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	texture->_type = GL_TEXTURE_CUBE_MAP;
	return texture;
}

Texture::~Texture() {
	if (_tex) {
		glDeleteTextures(1, &_tex);
		_tex = 0;
	}
}

GLuint Texture::getDefault(const GLenum type) {
	GLuint& tex = _defaults[type];
	if (!tex) {
		GLfloat data[4] = {0.0f, 0.0f, 0.0f, 0.0f};
		glCreateTextures(type, 1, &tex);
		if (type == GL_TEXTURE_2D) {
			glTextureStorage2D(tex, 1, GL_RGBA8, 1, 1);
			glTextureSubImage2D(tex, 0, 0, 0, 1, 1, GL_RGBA, GL_FLOAT, data);
		}
		else if (type == GL_TEXTURE_CUBE_MAP) {
			glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
			for (GLuint i = 0; i < 6; ++i){
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_FLOAT, data);
			}
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		}
	}

	return tex;
}

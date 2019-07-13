#include "texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

Texture::ptr Texture::create(const std::string& name, const std::filesystem::path& path) {
	Texture::ptr texture = std::shared_ptr<Texture>(new Texture());
	texture->setName(name);

	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(path.string().c_str(), &texture->_w, &texture->_h, &texture->_n, 4);
	if (!data) {
		std::cout << "stb image: " << path.string() << ", error: " << stbi_failure_reason() << std::endl;
		return {};
	}
	
	glGenTextures(1, &texture->_tex);
	glBindTexture(GL_TEXTURE_2D, texture->_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->_w, texture->_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);   
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(data);
	
	texture->_type = GL_TEXTURE_2D;
	return texture;
}

Texture::ptr Texture::create(const strcube& cube) {
	Texture::ptr texture = std::shared_ptr<Texture>(new Texture());

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

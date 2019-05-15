#include "texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

Texture::ptr Texture::create(const std::string& name) {
	Texture::ptr texture = std::shared_ptr<Texture>(new Texture());
	texture->setName(name);

	std::filesystem::path path = std::filesystem::current_path() / "resource" / "texture" / name;
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
	
	return texture;
}

Texture::~Texture() {
	if (_tex) {
		glDeleteTextures(1, &_tex);
		_tex = 0;
	}
}

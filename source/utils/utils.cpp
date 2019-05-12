#include "utils.hpp"

void oglFeature() {
	int nrAttributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	std::cout << "Maximum nr of vertex attributes supported: " << nrAttributes
	          << std::endl;
}

bool oglError() {
	bool ret = false;
	GLenum err;
	while((err = glGetError()) != GL_NO_ERROR)
	{
		ret = true;
		std::cout << "opengl error: " << err <<std::endl;
	}

	return ret;
}

bool Attributes::guessAttrs(const Config::node& doc) {
	if (!doc.IsMap()) {
		return false;
	}

	for (const auto& it: doc) {
		if (!it.first.IsScalar()) {
			std::cout << "attribute guess, key is not string, " << it.first.Mark().line << std::endl;
			return false;
		}
		const std::string& key = it.first.as<std::string>();
		std::any value = Config::guess(it.second);
		if (!value.has_value()) {
			std::cout << "attribute guess, not know, " << key << std::endl;
			return false;
		}
		setAttr(key, value);
	}

	return true;
}
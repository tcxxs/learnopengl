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

bool Config::load(const std::filesystem::path& path) {
	_doc = YAML::LoadFile(path.string());
	return _doc.IsDefined();
}

const Config::node Config::operator[](const std::string& path) const {
	size_t start = 0;
	size_t pos = 0;
	size_t len = path.size();
	Config::node node = _doc;
	while (start < len) {
		pos = path.find('.', start);
		if (pos == std::string::npos) {
			node.reset(node[path.substr(start, len - start)]);
			start = len;
		}
		else {
			node.reset(node[path.substr(start, pos - start)]);
			start = pos + 1;
		}		
		if (!node.IsDefined())
			return {};
	}

	return node;
}
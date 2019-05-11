#include "resource.hpp"

bool readFile(const std::filesystem::path& path, std::string& content) {
	std::fstream fs(path, std::ios::in | std::ios::binary | std::ios::ate);
	if (fs.fail()) {
		std::cout << "read file fail: " << path << std::endl;
		return false;
	}

	content.resize(fs.tellg());
	fs.seekg(0);
	fs.read(content.data(), content.size());
	return true;
}

bool Config::load(const std::filesystem::path& path) {
	_doc = YAML::LoadFile(path.string());
	return _doc.IsDefined();
}

const Config::node Config::visit(const Config::node& doc, const std::string& path) {
	size_t start = 0;
	size_t pos = 0;
	size_t len = path.size();
	Config::node node = doc;
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
#include "resource.hpp"

bool readFile(const std::filesystem::path& path, std::string& content) {
	std::fstream fs(path, std::ios::in | std::ios::binary | std::ios::ate);
	if (fs.fail()) {
		ERR("read file fail: %s", path.string().c_str());
		return false;
	}

	content.resize(fs.tellg());
	fs.seekg(0);
	fs.read(content.data(), content.size());
	return true;
}

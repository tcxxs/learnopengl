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

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

// yaml内部实现基于了大量的复制拷贝，如果引用，问题比较多
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
			return Config::_empty;
	}

	return node;
}

std::any Config::guess(const Config::node& doc) {
	if (doc.IsScalar()) {
		const std::string& scalar = doc.Scalar();
		if (std::isdigit(scalar[0]) || scalar[0] == '-') {
			if (scalar.find('.') == std::string::npos) {
				try {
					return {std::stoi(scalar)};
				} catch (...) {
					return {};
				}
			}
			else {
				try {
					return {std::stof(scalar)};
				} catch (...) {
					return {};
				}
			}
		}
		else
			return {scalar};
	}
	else if (doc.IsSequence()) {
		if (doc.size() == 3) {
			return {doc.as<glm::vec3>()};
		}
	}
	else
		return {};
}
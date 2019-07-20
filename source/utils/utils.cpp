#include "utils.hpp"

void oglFeature() {
	int val;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &val);
	std::cout << "GL_MAX_VERTEX_ATTRIBS: " << val << std::endl;
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &val);
	std::cout << "GL_MAX_VERTEX_UNIFORM_COMPONENTS: " << val << std::endl;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIB_BINDINGS, &val);
	std::cout << "GL_MAX_VERTEX_ATTRIB_BINDINGS: " << val << std::endl;
}

bool oglError() {
	bool ret = false;
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		ret = true;
		std::cout << "opengl error: " << err << std::endl;
	}

	return ret;
}

void APIENTRY oglDebug(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	// 忽略一些不重要的错误/警告代码
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
		return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source) {
	case GL_DEBUG_SOURCE_API: std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM: std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY: std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION: std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER: std::cout << "Source: Other"; break;
	}
	std::cout << std::endl;

	switch (type) {
	case GL_DEBUG_TYPE_ERROR: std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY: std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE: std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER: std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP: std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP: std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER: std::cout << "Type: Other"; break;
	}
	std::cout << std::endl;

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH: std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM: std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW: std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	}
	std::cout << std::endl;
	std::cout << std::endl;
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
			return Config::empty;
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
		else if (doc.size() == 6) {
			return {doc.as<strcube>()};
		}
	}

	return {};
}

bool Attributes::updateConf(const Config::node& doc) {
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
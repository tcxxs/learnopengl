#include "utils.hpp"

void oglFeature() {
	int val;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &val);
	INFO("GL_MAX_VERTEX_ATTRIBS: %d", val);
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &val);
	INFO("GL_MAX_VERTEX_UNIFORM_COMPONENTS: %d", val);
	glGetIntegerv(GL_MAX_VERTEX_ATTRIB_BINDINGS, &val);
	INFO("GL_MAX_VERTEX_ATTRIB_BINDINGS: %d", val);
}

bool oglError() {
	bool ret = false;
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		ret = true;
		ERR("opengl error: %d", err);
	}

	return ret;
}

void APIENTRY oglDebug(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	// 忽略一些不重要的错误/警告代码
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
		return;
	if (type == GL_DEBUG_TYPE_PUSH_GROUP || type == GL_DEBUG_TYPE_POP_GROUP)
		return;

	ERR("Debug message (%d): %s", id, message);

	switch (source) {
	case GL_DEBUG_SOURCE_API: ERR("Source: API"); break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM: ERR("Source: Window System"); break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: ERR("Source: Shader Compiler"); break;
	case GL_DEBUG_SOURCE_THIRD_PARTY: ERR("Source: Third Party"); break;
	case GL_DEBUG_SOURCE_APPLICATION: ERR("Source: Application"); break;
	case GL_DEBUG_SOURCE_OTHER: ERR("Source: Other"); break;
	}

	switch (type) {
	case GL_DEBUG_TYPE_ERROR: ERR("Type: Error"); break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: ERR("Type: Deprecated Behaviour"); break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: ERR("Type: Undefined Behaviour"); break;
	case GL_DEBUG_TYPE_PORTABILITY: ERR("Type: Portability"); break;
	case GL_DEBUG_TYPE_PERFORMANCE: ERR("Type: Performance"); break;
	case GL_DEBUG_TYPE_MARKER: ERR("Type: Marker"); break;
	case GL_DEBUG_TYPE_PUSH_GROUP: ERR("Type: Push Group"); break;
	case GL_DEBUG_TYPE_POP_GROUP: ERR("Type: Pop Group"); break;
	case GL_DEBUG_TYPE_OTHER: ERR("Type: Other"); break;
	}

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH: ERR("Severity: high"); break;
	case GL_DEBUG_SEVERITY_MEDIUM: ERR("Severity: medium"); break;
	case GL_DEBUG_SEVERITY_LOW: ERR("Severity: low"); break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: ERR("Severity: notification"); break;
	}
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
	// 由连续两层list组成
	if (doc.IsSequence() && doc.size() == 1 && doc[0].IsSequence()) {
		if (!gen) {
			std::printf("config not gen func");
			return {};
		}

		return gen(doc[0]);
	}

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
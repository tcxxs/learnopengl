#include "render.hpp"
#include "render/scene.hpp"
#include "render/material.hpp"
#include "render/vertex.hpp"

bool Render::init() {
	if (SystemMgr::inst().getDebug()) {
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(Render::onError, nullptr);
		//glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_ERROR, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
	}
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	if (SystemMgr::inst().getMSAA())
		glEnable(GL_MULTISAMPLE);

	if (!_initVertex())
		return false;
	if (!_initFeature())
		return false;
	return true;
}

bool Render::_initVertex() {
	if (!VertexProtoMgr::inst().req(VERTEX_BASE))
		return false;
	if (!VertexProtoMgr::inst().req(VERTEX_INSTANCE))
		return false;

	return true;
}

bool Render::_initFeature() {
	int val;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &val);
	INFO("GL_MAX_VERTEX_ATTRIBS: %d", val);
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &val);
	INFO("GL_MAX_VERTEX_UNIFORM_COMPONENTS: %d", val);
	glGetIntegerv(GL_MAX_VERTEX_ATTRIB_BINDINGS, &val);
	INFO("GL_MAX_VERTEX_ATTRIB_BINDINGS: %d", val);
	return true;
}

void APIENTRY Render::onError(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
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

void Render::onRender() {
	if (Scene::current) {
		Scene::current->draw();
	}
}

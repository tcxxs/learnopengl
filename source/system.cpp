#include "System.hpp"
#include "render/scene.hpp"
#include "ui/ui.hpp"

System::~System() {
	glfwTerminate();
}

bool System::init() {
	if (!_initConfig())
		return false;
	if (!_initWindow())
		return false;

	return true;
}

bool System::_initConfig() {
	std::filesystem::path path = std::filesystem::current_path() / "resource" / "config.yml";
	Config conf;
	if (!conf.load(path)) {
		ERR("render config error");
		return false;
	}

	Logger::init(conf["log"]);
	_debug = conf["debug"].as<bool>(false);
	_fps = conf["fps"].as<int>(60);
	_width = conf["width"].as<int>(1024);
	_height = conf["height"].as<int>(768);
	_bgcolor = conf["bgcolor"].as<glm::vec3>(glm::vec3(0.0f));
	_msaa = conf["msaa"].as<int>(0);
	_scene = conf["scene"].as<std::string>();
	return true;
}

bool System::_initWindow() {
	glfwSetErrorCallback(System::onError);
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	if (_debug)
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	if (_msaa > 0)
		glfwWindowHint(GLFW_SAMPLES, _msaa);

	_window = glfwCreateWindow(_width, _height, "LearnOpenGL", nullptr, nullptr);
	if (_window == nullptr) {
		ERR("Failed to create GLFW window");
		return false;
	}
	//glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwMakeContextCurrent(_window);
	glfwSetFramebufferSizeCallback(_window, [](GLFWwindow* window, int width, int height) { SystemMgr::inst().onResize(width, height); });
	glfwSetMouseButtonCallback(_window, [](GLFWwindow* window, int button, int action, int mods) { SystemMgr::inst().onMouse(button, action); });
	glfwSetCursorPosCallback(_window, [](GLFWwindow* window, double xpos, double ypos) { SystemMgr::inst().onCursor((float)xpos, (float)ypos); });
	glfwSetScrollCallback(_window, [](GLFWwindow* window, double xoffset, double yoffset) { SystemMgr::inst().onScroll((float)xoffset, (float)yoffset); });

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		ERR("Failed to initialize GLAD");
		return false;
	}

	float now = (float)glfwGetTime();
	_time_last = now;
	_frame_last = now;
	_frame_interv = 1.0f / _fps;
	return true;
}

void System::onResize(int width, int height) {
	// TODO: 应该调整width、height，以及所有framebuffer
	glViewport(0, 0, width, height);
}

void System::process() {
	float now;
	while (!glfwWindowShouldClose(_window)) {
		now = (float)glfwGetTime();
		_time_delta = now - _time_last;
		_time_last = now;
		_frame_delta = now - _frame_last;

		onInput();
		if (_frame_delta > _frame_interv) {
			_frame_last = now;
			RenderMgr::inst().onRender();
			UIMgr::inst().onRender();
			glfwSwapBuffers(_window);
		}

		glfwPollEvents();
	}
}

bool System::_onClick(int key) {
	bool press = glfwGetKey(_window, key) == GLFW_PRESS;
	auto& find = _keys.find(key);
	if (find == _keys.end()) {
		_keys[key] = press;
		return false;
	}
	else {
		bool click = find->second && !press;
		find->second = press;
		return click;
	}
}

void System::onInput() {
	if (_onClick(GLFW_KEY_ESCAPE)) {
		if (_cursor) {
			glfwSetWindowShouldClose(_window, true);
		}
		else {
			_cursor = true;
			glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}
	if (_onClick(GLFW_KEY_ENTER)) {
		if (_cursor) {
			_cursor = false;
			glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
	}

	if (_cursor)
		return;
	if (!Scene::current)
		return;

	auto& cam = Scene::current->getCamera();
	if (glfwGetKey(_window, GLFW_KEY_W) == GLFW_PRESS)
		cam->setPos(cam->getPos() - CAM_MOVE * _time_delta * cam->getFront());
	if (glfwGetKey(_window, GLFW_KEY_S) == GLFW_PRESS)
		cam->setPos(cam->getPos() + CAM_MOVE * _time_delta * cam->getFront());
	if (glfwGetKey(_window, GLFW_KEY_A) == GLFW_PRESS)
		cam->setPos(cam->getPos() - CAM_MOVE * _time_delta * cam->getRight());
	if (glfwGetKey(_window, GLFW_KEY_D) == GLFW_PRESS)
		cam->setPos(cam->getPos() + CAM_MOVE * _time_delta * cam->getRight());
}

void System::onMouse(int button, int action) {
	if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		if (action == GLFW_PRESS) {
			_cursor = false;
			glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		else {
			_cursor = true;
			glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}
}

void System::onCursor(float xpos, float ypos) {
	if (_cursor)
		return;
	if (!Scene::current)
		return;

	if (!_mouse_init) {
		_mouse_init = true;
		_mouse_x = xpos;
		_mouse_y = ypos;
		return;
	}

	float xoffset = (xpos - _mouse_x) * CAM_ROTATE;
	float yoffset = (_mouse_y - ypos) * CAM_ROTATE;
	_mouse_x = xpos;
	_mouse_y = ypos;

	auto& cam = Scene::current->getCamera();
	cam->setYaw(cam->getYaw() + xoffset);
	cam->setPitch(cam->getPitch() - yoffset);
}

void System::onScroll(float xoffset, float yoffset) {
	if (_cursor)
		return;
	if (!Scene::current)
		return;

	auto& cam = Scene::current->getCamera();
	cam->setFov(cam->getFov() + yoffset * CAM_FOV);
}
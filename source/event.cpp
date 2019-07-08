#include "event.hpp"
#include "render/scene.hpp"

Event::~Event() {
	glfwTerminate();
}

bool Event::init() {
	if (!_initConfig())
		return false;
	if (!_initWindow())
		return false;
}

bool Event::_initConfig() {
	std::filesystem::path path = std::filesystem::current_path() / "resource" / "config.yml";
	Config conf;
	if (!conf.load(path)) {
		std::cout << "render config error";
		return false;
	}

	_fps = conf["fps"].as<int>(60);
	_width = conf["width"].as<int>(1024);
	_height = conf["height"].as<int>(768);
	_bgcolor = conf["bgcolor"].as<glm::vec3>(glm::vec3(0.0f));
	_msaa = conf["msaa"].as<int>(0);
	return true;
}

bool Event::_initWindow() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	if (_msaa > 0)
		glfwWindowHint(GLFW_SAMPLES, _msaa);

	_window = glfwCreateWindow(_width, _height, "LearnOpenGL", NULL, NULL);
	if (_window == nullptr) {
		std::cout << "Failed to create GLFW window" << std::endl;
		return false;
	}
	glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwMakeContextCurrent(_window);
	glfwSetFramebufferSizeCallback(_window,
	                               [](GLFWwindow* window, int width, int height) {
		                               EventMgr::inst().onResize(width, height);
	                               });
	glfwSetCursorPosCallback(_window,
	                         [](GLFWwindow* window, double xpos, double ypos) {
		                         EventMgr::inst().onMouse((float)xpos, (float)ypos);
	                         });
	glfwSetScrollCallback(_window,
	                      [](GLFWwindow* window, double xoffset, double yoffset) {
		                      EventMgr::inst().onScroll((float)xoffset, (float)yoffset);
	                      });

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}

	float now = (float)glfwGetTime();
	_time_last = now;
	_frame_last = now;
	_frame_interv = 1.0f / _fps;
	return true;
}

void Event::onResize(int width, int height) {
	glViewport(0, 0, width, height);
}

void Event::process() {
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
			glfwSwapBuffers(_window);
		}

		glfwPollEvents();
	}
}

void Event::onInput() {
	if (glfwGetKey(_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(_window, true);

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

void Event::onMouse(float xpos, float ypos) {
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

void Event::onScroll(float xoffset, float yoffset) {
	auto& cam = Scene::current->getCamera();
	cam->setFov(cam->getFov() + yoffset * CAM_FOV);
}
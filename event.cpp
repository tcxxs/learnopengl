#include "event.hpp"

void onResize(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void Event::init(GLFWwindow* window, int fps) {
	_window = window;

	float now = (float)glfwGetTime();
	_time_last = now;
	_frame_last = now;
	_frame_interv = 1.0f / fps;

	glfwSetCursorPosCallback(window,
	                         [](GLFWwindow* window, double xpos, double ypos) {
		                         EventMgr::inst().onMouse((float)xpos, (float)ypos);
	                         });
}

void Event::onUpdate() {
	float now = (float)glfwGetTime();
	_time_delta = now - _time_last;
	_time_last = now;
	_frame_delta = now - _frame_last;

	onInput();
	if (_frame_delta > _frame_interv) {
		_frame_last = now;
		RenderMgr::inst().onRender();
		glfwSwapBuffers(_window);
	}
}

void Event::onInput() {
	if (glfwGetKey(_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(_window, true);

	const Camera::ptr& cam = RenderMgr::inst().getCamera();
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

	const Camera::ptr& cam = RenderMgr::inst().getCamera();
	cam->setYaw(cam->getYaw() + xoffset);
	cam->setPitch(cam->getPitch() - yoffset);
}

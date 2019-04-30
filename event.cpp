#include "event.hpp"
#include "glm/gtx/string_cast.hpp"

void onResize(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void Event::init(GLFWwindow* window, int fps) {
    _window = window;
    float now = glfwGetTime();
    _time_last = now;
    _frame_last = now;
    _frame_interv = 1.0f / fps;
}

void Event::onUpdate() {
    float now = glfwGetTime();
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

void Event::onInput()
{
	if (glfwGetKey(_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(_window, true);

    float speed = 0.5f * _time_delta;
	const Camera::ptr& cam = RenderMgr::inst().getCamera();
	if (glfwGetKey(_window, GLFW_KEY_W) == GLFW_PRESS)
		cam->setPos(cam->getPos() - speed * cam->getFront());
    if (glfwGetKey(_window, GLFW_KEY_S) == GLFW_PRESS)
        cam->setPos(cam->getPos() + speed * cam->getFront());
    if (glfwGetKey(_window, GLFW_KEY_A) == GLFW_PRESS)
        cam->setPos(cam->getPos() - speed * cam->getRight());
    if (glfwGetKey(_window, GLFW_KEY_D) == GLFW_PRESS)
        cam->setPos(cam->getPos() + speed * cam->getRight());
}
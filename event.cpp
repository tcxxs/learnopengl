#include "event.hpp"

void onResize(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void onInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

bool oglError() {
	bool ret = false;
	GLenum err;
	while((err = glGetError()) != GL_NO_ERROR)
	{
		ret = true;
		std::cout << "opengl error: " << err <<std::endl;
	}

	return ret;
}

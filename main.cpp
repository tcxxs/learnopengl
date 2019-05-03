#include <iostream>

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "GLFW/glfw3.h"
#include "config.hpp"
#include "event.hpp"
#include "utils/resource.hpp"
#include "render/render.hpp"
#include "render/model.hpp"
#include "render/shader.hpp"
#include "render/texture.hpp"

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, onResize);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	Shader::ptr&& shader = Shader::create("simple");
	if (!shader) {
		return -1;
	}
	ShaderMgr::inst().add("simple", shader);

	Texture::ptr&& texture = Texture::create("oops.png");
	if (!texture) {
		return -1;
	}
	TextureMgr::inst().add("oops", texture);

	Mesh::ptr mesh = Mesh::create(CUBE_VERTEX, CUBE_INDEX);
	if (!mesh) {
		return -1;
	}

	Model::ptr model = Model::create(mesh, shader, texture);
	if (!model) {
		return -1;
	}
	model->setVar("lcolor", glm::vec3(1.0f, 1.0f, 1.0f));
	ModelMgr::inst().add("test", model);

	Model::ptr light = Model::create(mesh, shader, nullptr);
	if (!light) {
		return -1;
	}
    glm::mat4 mat{1.0f};
    mat = glm::translate(mat, glm::vec3(2.0f, 2.0f, -1.0f));
    light->setMatrix(mat);
	ModelMgr::inst().add("light", light);

    EventMgr::inst().init(window, FPS);
	RenderMgr::inst().init();
	while (!glfwWindowShouldClose(window))
	{
        EventMgr::inst().onUpdate();
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
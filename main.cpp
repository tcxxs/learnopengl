#include <iostream>

#include "glad/glad.h"
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

    GLFWwindow* window = glfwCreateWindow(WIDTH, HIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, WIDTH, HIGHT);
	glfwSetFramebufferSizeCallback(window, onResize);

	oglFeature();

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

	std::vector<GLfloat> verts = {
//     ---- 位置 ----       ---- 颜色 ----     - 纹理坐标 -
     0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // 右上
     0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // 右下
    -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // 左下
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // 左上
	};
	std::vector<GLuint> inds = { // 注意索引从0开始! 
    0, 1, 3, // 第一个三角形
    1, 2, 3  // 第二个三角形
	};
	Mesh::ptr mesh = Mesh::create(verts, inds);
	if (!mesh) {
		return -1;
	}

	Model::ptr model = Model::create(mesh, shader, texture);
	if (!model) {
		return -1;
	}
	ModelMgr::inst().add("test", model);

	while (!glfwWindowShouldClose(window))
	{
		onInput(window);

		onRender();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
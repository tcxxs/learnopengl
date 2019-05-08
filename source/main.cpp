#include <iostream>

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "GLFW/glfw3.h"
#include "config.hpp"
#include "event.hpp"
#include "utils/resource.hpp"
#include "render/render.hpp"
#include "render/mesh.hpp"
#include "render/model.hpp"
#include "render/shader.hpp"
#include "render/texture.hpp"

int main()
{
    if (!EventMgr::inst().init(FPS)) {
		return -1;
	}
	RenderMgr::inst().init();


	// Shader::ptr simple = Shader::create("simple");
	// ShaderMgr::inst().add("simple", simple);
	Shader::ptr color = Shader::create("color");
	if (!color) return -1;
	ShaderMgr::inst().add("color", color);

	Mesh::ptr cube = Mesh::create(CUBE_VERTEX, CUBE_INDEX);
	MeshMgr::inst().add("cube", cube);

	Model::ptr test = Model::create("test");
	if (!test)
		return -1;
	test->setVar("light.pos", glm::vec3(-1.2f, 1.2f, -0.5f));
	test->setVar("light.ambient", glm::vec3(0.2f));
	test->setVar("light.diffuse", glm::vec3(0.5f));
	test->setVar("light.specular", glm::vec3(1.0f));
	test->setVar("material.ambient", glm::vec3(1.0f, 0.5f, 0.31f));
	test->setVar("material.diffuse", glm::vec3(1.0f, 0.5f, 0.31f));
	test->setVar("material.specular", glm::vec3(0.5f, 0.5f, 0.5f));
	test->setVar("material.shininess", 32.0f);
	ModelMgr::inst().add("test", test);

	Model::ptr light = Model::create(cube, color, nullptr);
    glm::mat4 mat{1.0f};
	mat = glm::translate(mat, glm::vec3(-1.2f, 1.2f, -0.5f));
	mat = glm::scale(mat, glm::vec3(0.2f));
    light->setMatrix(mat);
	light->setVar("uf_color", glm::vec3(1.0f));
	ModelMgr::inst().add("light", light);

	EventMgr::inst().process();
	return 0;
}
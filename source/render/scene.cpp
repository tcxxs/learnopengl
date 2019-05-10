#include "scene.hpp"

Scene::ptr Scene::create(const std::string& name) {
	Scene::ptr scene = std::shared_ptr<Scene>(new Scene());
	std::filesystem::path path = std::filesystem::current_path() / "resource" / "scene" / (name + ".yml");
	if (!scene->_conf.load(path)) {
		std::cout << "scene config error, " << path << std::endl;
		return {};
	}

	const auto& node = scene->_conf["models"];
	if (node.IsDefined()) {
		for (const auto& it: node) {
			scene->addModel(it);
		}
	}
	
	return scene;
}

void Scene::addModel(const Config::node& conf) {
	Model::ptr model = ModelMgr::inst().req(conf["conf"].as<std::string>());
	glm::mat4 mat{1.0f};
	mat = glm::translate(mat, conf["pos"].as<glm::vec3>());
	model->setMatrix(mat);
	
	_models[conf["name"].as<std::string>()] = model;
}
#include "scene.hpp"

Scene::ptr Scene::create(const std::string& name) {
	Scene::ptr scene = std::shared_ptr<Scene>(new Scene());
	std::filesystem::path path = std::filesystem::current_path() / "resource" / "scene" / (name + ".yml");
	if (!scene->_conf.load(path)) {
		std::cout << "scene config error, " << path << std::endl;
		return {};
	}

	scene->addCamera(scene->_conf["camera"]);

	const auto& models = scene->_conf["models"];
	if (models.IsDefined()) {
		for (const auto& it: models) {
			scene->addModel(it);
		}
	}

	const auto& lights = scene->_conf["lights"];
	if (lights.IsDefined()) {
		for (const auto& it: lights) {
			scene->addLight(it);
		}
	}

	return scene;
}

Scene::~Scene() {
	
}

void Scene::addCamera(const Config::node& conf) {
	_cam = Camera::create();
	_cam->setFov(conf["fov"].as<float>());
	_cam->lookAt(conf["pos"].as<glm::vec3>(), conf["target"].as<glm::vec3>());
}

void Scene::addModel(const Config::node& conf) {
	Model::ptr model = ModelMgr::inst().req(conf["conf"].as<std::string>());
	glm::mat4 mat{1.0f};
	mat = glm::translate(mat, conf["pos"].as<glm::vec3>());
	model->setMatrix(mat);
	
	_models[conf["name"].as<std::string>()] = model;
}

void Scene::addLight(const Config::node& conf) {
	Light::ptr light = Light::create();
	light->setPos(conf["pos"].as<glm::vec3>());
	light->setColor(conf["color"].as<glm::vec3>());
	
	_lights[conf["name"].as<std::string>()] = light;
}

void Scene::draw() {
	const glm::vec3& cam_pos = _cam->getPos();
	const glm::mat4& view = _cam->getView();
	const glm::mat4& proj = _cam->getProj();

	for (auto& it: _models) {
		const Model::ptr& model = it.second;
		model->setVar("camera_pos", cam_pos);
		model->draw(view, proj);
	}
}
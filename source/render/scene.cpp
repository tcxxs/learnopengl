#include "scene.hpp"

Scene::ptr Scene::create(const std::string& name) {
	Scene::ptr scene = std::shared_ptr<Scene>(new Scene());
	std::filesystem::path path = std::filesystem::current_path() / "resource" / "scene" / (name + ".yml");
	if (!scene->_conf.load(path)) {
		std::cout << "scene config error, " << path << std::endl;
		return {};
	}

	scene->addCamera(scene->_conf["camera"]);

	const auto models = scene->_conf["models"];
	if (models.IsDefined()) {
		for (const auto& it: models) {
			scene->addModel(it);
		}
	}

	const auto lights = scene->_conf["lights"];
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
	glm::vec3 pos = conf["pos"].as<glm::vec3>();
	glm::vec3 tar = conf["target"].as<glm::vec3>();
	_cam->lookAt(pos, tar);
}

void Scene::addModel(const Config::node& conf) {
	const Model::ptr& model = ModelMgr::inst().req(conf["conf"].as<std::string>());
	glm::mat4 mat{1.0f};
	mat = glm::translate(mat, conf["pos"].as<glm::vec3>());
	const Config::node scale = conf["scale"];
	if (scale.IsDefined()) {
		mat = glm::scale(mat, glm::vec3(scale.as<float>()));
	}
	model->setMatrix(mat);
	
	_models[conf["name"].as<std::string>()] = model;
}

void Scene::addLight(const Config::node& conf) {
	const Light::ptr& light = LightMgr::inst().req(conf["conf"].as<std::string>());
	light->setPos(conf["pos"].as<glm::vec3>());
	
	_lights[conf["name"].as<std::string>()] = light;
}

void Scene::draw() {
	const glm::vec3& cam_pos = _cam->getPos();
	const glm::mat4& view = _cam->getView();
	const glm::mat4& proj = _cam->getProj();

	const glm::vec3& light_pos = _lights.begin()->second->getPos();
	const glm::vec3& light_ambient = _lights.begin()->second->attrs.getAttr<glm::vec3>("ambient");
	const glm::vec3& light_diffuse = _lights.begin()->second->attrs.getAttr<glm::vec3>("diffuse");
	const glm::vec3& light_specular = _lights.begin()->second->attrs.getAttr<glm::vec3>("specular");

	for (auto& it: _models) {
		const Model::ptr& model = it.second;
		model->attrs.setAttr("camera_pos", cam_pos);
		model->attrs.setAttr("light.pos", light_pos);
		model->attrs.setAttr("light.ambient", light_ambient);
		model->attrs.setAttr("light.diffuse", light_diffuse);
		model->attrs.setAttr("light.specular", light_specular);
		model->draw(view, proj);
	}
}
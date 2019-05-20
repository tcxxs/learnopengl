#include "scene.hpp"

Scene::ptr Scene::create(const std::string& name) {
	Scene::ptr scene = std::shared_ptr<Scene>(new Scene());
	scene->setName(name);

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
	const ModelProto::ptr& proto = ModelProtoMgr::inst().req(conf["type"].as<std::string>());
	if (!proto)
		return;
	const ModelInst::ptr& model = proto->instance(conf);
	if (!model)
		return;
	
	_models[conf["type"].as<std::string>()] = proto;
}

void Scene::addLight(const Config::node& conf) {
	const LightProto::ptr& proto = LightProtoMgr::inst().req(conf["type"].as<std::string>());
	if (!proto)
		return;
	const LightInst::ptr& light = proto->instance(conf);
	if (!light)
		return;
	
	_lights[conf["type"].as<std::string>()] = proto;
}

void Scene::draw() {
	for (auto& it: _models) {
		const ModelProto::ptr& proto = it.second;
		proto->draw(_cam, _lights);
	}
}
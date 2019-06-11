#include "scene.hpp"
#include "render/command.hpp"

Scene::ptr Scene::create(const std::string& name) {
	Scene::ptr scene = std::shared_ptr<Scene>(new Scene());
	scene->setName(name);

	std::filesystem::path path = std::filesystem::current_path() / "resource" / "scene" / (name + ".yml");
	if (!scene->_conf.load(path)) {
		std::cout << "scene config error, " << path << std::endl;
		return {};
	}

	scene->addCamera(scene->_conf["camera"]);

	const auto lights = scene->_conf["lights"];
	if (lights.IsDefined()) {
		for (const auto& it: lights) {
			scene->addLight(it);
		}
	}

	const auto models = scene->_conf["models"];
	for (const auto& it: models) {
		scene->addModel(it);
	}

	const auto pass = scene->_conf["pass"];
	for (const auto& it: pass) {
		//scene->addPass(it);
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

void Scene::addLight(const Config::node& conf) {
	const LightProto::ptr& proto = LightProtoMgr::inst().req(conf["type"].as<std::string>());
	if (!proto)
		return;
	const LightInst::ptr& light = proto->instance(conf);
	if (!light)
		return;

	_lights.push_back(light);
}

void Scene::addModel(const Config::node& conf) {
	const ModelProto::ptr& proto = ModelProtoMgr::inst().req(conf["type"].as<std::string>());
	if (!proto)
		return;
	const ModelInst::ptr& model = proto->instance(conf);
	if (!model)
		return;

	_models.push_back(model);
}

void Scene::addPass(const Config::node& conf) {
	conf["name"].as<std::string>();
	const std::string& in = conf["in"].as<std::string>();
	if (!in.empty()) {

		Frame::ptr outf = Frame::create();
		outf->attachTexture();
		_frames[in] = outf;
		
	}
	const std::string& out = conf["out"].as<std::string>();
	if (!out.empty()) {
		Frame::ptr outf = Frame::create();
		outf->attachTexture();
		_frames[out] = outf;
	}
	const std::string& modelsk = conf["models"].as<std::string>();
	if (!modelsk.empty()) {
	}

}

void Scene::draw() {
	CommandQueue cmds;
	for (auto& it: _models) {
		it->draw(cmds);
	}

	for (auto& it: cmds) {
		drawCommand(it);
	}
}

void Scene::drawCommand(const Command& cmd) {
	cmd.material->use();
	const Shader::ptr& shader = cmd.material->getShader();
	shader->setVars(cmd.attrs);

	glUniformMatrix4fv(shader->getVar("view"), 1, GL_FALSE, glm::value_ptr(_cam->getView()));
	glUniformMatrix4fv(shader->getVar("proj"), 1, GL_FALSE, glm::value_ptr(_cam->getProj()));
	glUniformMatrix4fv(shader->getVar("model"), 1, GL_FALSE, glm::value_ptr(cmd.model));
	shader->setVar("camera_pos", _cam->getPos());

	int dirs{0}, points{0}, spots{0};
	std::string light;
	for (const auto& it: _lights) {
		const LightProto::ptr proto = it->prototype();
		const std::string& light_type = proto->getName();
		if (light_type == "dir") {
			light = string_format("dirs[%d]", dirs++);
			shader->setVar(light + ".dir", it->getDir());
			shader->setVar(light + ".ambient", proto->attrs.getAttr<glm::vec3>("ambient"));
			shader->setVar(light + ".diffuse", proto->attrs.getAttr<glm::vec3>("diffuse"));
			shader->setVar(light + ".specular", proto->attrs.getAttr<glm::vec3>("specular"));
		}
		else if (light_type == "point") {
			light = string_format("points[%d]", points++);
			shader->setVar(light + ".pos", it->getPos());
			shader->setVar(light + ".ambient", proto->attrs.getAttr<glm::vec3>("ambient"));
			shader->setVar(light + ".diffuse", proto->attrs.getAttr<glm::vec3>("diffuse"));
			shader->setVar(light + ".specular", proto->attrs.getAttr<glm::vec3>("specular"));
			shader->setVar(light + ".constant", proto->attrs.getAttr<float>("constant"));
			shader->setVar(light + ".linear", proto->attrs.getAttr<float>("linear"));
			shader->setVar(light + ".quadratic", proto->attrs.getAttr<float>("quadratic"));
		}
		else if (light_type == "spot") {
			light = string_format("spots[%d]", spots++);
			shader->setVar(light + ".pos", it->getPos());
			shader->setVar(light + ".dir", it->getDir());
			shader->setVar(light + ".ambient", proto->attrs.getAttr<glm::vec3>("ambient"));
			shader->setVar(light + ".diffuse", proto->attrs.getAttr<glm::vec3>("diffuse"));
			shader->setVar(light + ".specular", proto->attrs.getAttr<glm::vec3>("specular"));
			shader->setVar(light + ".constant", proto->attrs.getAttr<float>("constant"));
			shader->setVar(light + ".linear", proto->attrs.getAttr<float>("linear"));
			shader->setVar(light + ".quadratic", proto->attrs.getAttr<float>("quadratic"));
			shader->setVar(light + ".inner", cos(glm::radians(proto->attrs.getAttr<float>("inner"))));
			shader->setVar(light + ".outter", cos(glm::radians(proto->attrs.getAttr<float>("outter"))));
		}
	}
	shader->setVar("uses.dirs", dirs);
	shader->setVar("uses.points", points);
	shader->setVar("uses.spots", spots);

	glBindVertexArray(cmd.vao);
	glDrawElements(GL_TRIANGLES, cmd.ibosize, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
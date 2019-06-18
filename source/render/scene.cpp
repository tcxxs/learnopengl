#include "scene.hpp"
#include "render/command.hpp"

Scene::ptr Scene::create(const std::string& name) {
	Scene::ptr scene = std::shared_ptr<Scene>(new Scene());
	scene->setName(name);

	Config conf;
	std::filesystem::path path = std::filesystem::current_path() / "resource" / "scene" / (name + ".yml");
	if (!conf.load(path)) {
		std::cout << "scene config error, " << path << std::endl;
		return {};
	}

	scene->addCamera(conf["camera"]);

	const auto lights = conf["lights"];
	if (lights.IsDefined()) {
		for (const auto& it: lights) {
			if (!scene->addLight(it))
				return {};
		}
	}

	const auto models = conf["models"];
	for (const auto& it: models) {
		if (!scene->addModel(it))
			return {};
	}

	const auto pass = conf["pass"];
	for (const auto& it: pass) {
		if (!scene->addPass(it))
			return {};
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

bool Scene::addLight(const Config::node& conf) {
	const LightProto::ptr& proto = LightProtoMgr::inst().req(conf["type"].as<std::string>());
	if (!proto)
		return false;
	const LightInst::ptr& light = proto->instance(conf);
	if (!light)
		return false;

	_lights.push_back(light);
	return true;
}

bool Scene::addModel(const Config::node& conf) {
	const ModelProto::ptr& proto = ModelProtoMgr::inst().req(conf["type"].as<std::string>());
	if (!proto)
		return false;
	const ModelInst::ptr& model = proto->instance(conf);
	if (!model)
		return false;

	_models.push_back(model);
	return true;
}

bool Scene::addPass(const Config::node& conf) {
	Pass& pass = _pass.emplace_back();

	pass.name = conf["name"].as<std::string>();
	if (!conf["post"].IsNull()) {
		pass.post = PostMgr::inst().req(conf["post"].as<std::string>());
		if (!pass.post)
			return false;
	}

	if (!conf["in"].IsNull()) {
		for (auto& it: conf["in"]) {
			auto find = _frames.find(it.as<std::string>());
			if (find == _frames.end()) {
				std::cout << "pass input not found, " << it.as<std::string>();
				return false;
			}
			pass.ins.push_back(find->second);
		}
	}
	if (!conf["out"].IsNull()) {
		Frame::ptr frame = Frame::create();
		frame->attachTexture();
		frame->attachDepthStencil();
		_frames[conf["out"].as<std::string>()] = frame;
		pass.out = frame;
	}
	for (const auto& it: conf["state"]) {
		pass.states.insert(it.as<std::string>());
	}

	return true;
}

void Scene::draw() {
	for (const auto& it: _pass) {
		glViewport(0, 0, WIDTH, HEIGHT);
		it.states.count("depth") ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glEnable(GL_CULL_FACE);

		if (it.out) {
			it.out->useBegin();
		}
		if (it.states.count("clear")) {
			glClearColor(BG_COLOR);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		}

		if (it.post) {
			drawPost(it);
		}
		else {
			drawScene(it);
		}
		if (it.out) {
			it.out->useEnd();
		}
	}
}

void Scene::drawScene(const Pass& pass) {
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
	if (cmd.ibosize > 0)
		glDrawElements(GL_TRIANGLES, cmd.ibosize, GL_UNSIGNED_INT, 0);
	else
		glDrawArrays(GL_TRIANGLES, 0, cmd.arrsize);
	glBindVertexArray(0);
}

void Scene::drawPost(const Pass& pass) {
	pass.post->draw(pass.ins);
}

#include "scene.hpp"
#include "event.hpp"
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

	for (const auto& it: conf["cameras"]) {
		const std::string& name = it.first.as<std::string>();
		scene->addCamera(name, it.second);
	}
	const std::string cam = conf["camera"].as<std::string>();
	const auto& find = scene->_cams.find(cam);
	if (find == scene->_cams.end()) {
		std::cout << "scene camera not found, " << cam << std::endl;
		return {};
	}
	scene->_cam = find->second;

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

	if (!scene->addUniform(UNIFORM_MATVP))
		return {};
	if (!scene->addUniform(UNIFORM_SCENE))
		return {};
	if (!scene->addUniform(UNIFORM_LIGHTS, LIGHT_MAX))
		return {};

	return scene;
}

Scene::~Scene() {
}

void Scene::addCamera(const std::string& name, const Config::node& conf) {
	Camera::ptr cam = Camera::create();
	cam->setFov(conf["fov"].as<float>());
	glm::vec3 pos = conf["pos"].as<glm::vec3>();
	glm::vec3 tar = conf["target"].as<glm::vec3>();
	cam->lookAt(pos, tar);
	_cams[name] = cam;
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
	Pass::ptr pass = Pass::create(conf, _frames);
	if (!pass)
		return false;

	_pass.push_back(pass);
	return true;
}

bool Scene::addUniform(const std::string& name, int count) {
	const UniformProto::ptr& proto = UniformProtoMgr::inst().get(name);
	if (!proto)
		return false;

	std::string uname{name};
	for (int i = 0; i < count; ++i) {
		if (count > 1)
			uname = name + string_format("[%d]", i);
		UniformInst::ptr uniform = proto->instance();
		if (!uniform)
			return false;
		_uniforms.emplace(uname, uniform);
	}
	return true;
}

// TODO: skybox画之前的gl call有200多个
// uniform data: 场景的uniform在sky pass也设置了
// vertex array: mesh的bind vertex也做了
void Scene::draw() {
	for (const auto& it: _pass) {
		glViewport(0, 0, EventMgr::inst().getWidth(), EventMgr::inst().getHeight());
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glEnable(GL_CULL_FACE);

		it->drawBegin();
		if (!it->drawPost())
			drawScene(it);
		it->drawEnd();
	}
}

void Scene::drawScene(const Pass::ptr& pass) {
	Camera::ptr cam = _cam;
	const auto& find = _cams.find(pass->getCamera());
	if (find != _cams.end())
		cam = find->second;
	
	UniformInst::ptr& matvp = _uniforms[UNIFORM_MATVP];
	matvp->setVar("view", cam->getView());
	matvp->setVar("proj", cam->getProj());

	for (int i = 0; i < _lights.size(); ++i) {
		UniformInst::ptr& uniform = _uniforms[string_format(UNIFORM_LIGHTS "[%d]", i)];
		const LightInst::ptr& light = _lights[i];
		const LightProto::ptr& proto = light->prototype();
		const int type = proto->getType();

		uniform->setVar("light.type", type);
		switch (proto->getType()) {
		case LIGHT_DIR:
			uniform->setVar("light.dir", light->getDir());
			uniform->setVar("light.ambient", proto->attrs.getAttr<glm::vec3>("ambient"));
			uniform->setVar("light.diffuse", proto->attrs.getAttr<glm::vec3>("diffuse"));
			uniform->setVar("light.specular", proto->attrs.getAttr<glm::vec3>("specular"));
			break;
		case LIGHT_POINT:
			uniform->setVar("light.pos", light->getPos());
			uniform->setVar("light.ambient", proto->attrs.getAttr<glm::vec3>("ambient"));
			uniform->setVar("light.diffuse", proto->attrs.getAttr<glm::vec3>("diffuse"));
			uniform->setVar("light.specular", proto->attrs.getAttr<glm::vec3>("specular"));
			uniform->setVar("light.constant", proto->attrs.getAttr<float>("constant"));
			uniform->setVar("light.linear", proto->attrs.getAttr<float>("linear"));
			uniform->setVar("light.quadratic", proto->attrs.getAttr<float>("quadratic"));
			break;
		case LIGHT_SPOT:
			uniform->setVar("light.pos", light->getPos());
			uniform->setVar("light.dir", light->getDir());
			uniform->setVar("light.ambient", proto->attrs.getAttr<glm::vec3>("ambient"));
			uniform->setVar("light.diffuse", proto->attrs.getAttr<glm::vec3>("diffuse"));
			uniform->setVar("light.specular", proto->attrs.getAttr<glm::vec3>("specular"));
			uniform->setVar("light.constant", proto->attrs.getAttr<float>("constant"));
			uniform->setVar("light.linear", proto->attrs.getAttr<float>("linear"));
			uniform->setVar("light.quadratic", proto->attrs.getAttr<float>("quadratic"));
			uniform->setVar("light.inner", cos(glm::radians(proto->attrs.getAttr<float>("inner"))));
			uniform->setVar("light.outter", cos(glm::radians(proto->attrs.getAttr<float>("outter"))));
			break;
		}
	}

	UniformInst::ptr& scene = _uniforms[UNIFORM_SCENE];
	scene->setVar("camera", cam->getPos());
	scene->setVar("lights", (int)_lights.size());

	CommandQueue cmds;
	for (auto& it: _models) {
		if (it->draw(cmds) < 0) {
			continue;
		}
	}

	const auto& shaders = pass->getShaders();
	for (auto& it: cmds) {
		if (shaders.empty() || shaders.count(it.material->getShader()) > 0)
			drawCommand(it);
	}
}

void Scene::drawCommand(const Command& cmd) {
	cmd.material->use();
	const Shader::ptr& shader = cmd.material->getShader();

	shader->setVars(cmd.attrs);
	if (!cmd.ins) {
		shader->setVar("model", cmd.model);
	}

	GLint point;
	point = shader->getVar(UNIFORM_MATVP);
	if (point >= 0)
		_uniforms[UNIFORM_MATVP]->bind(point);
	point = shader->getVar(UNIFORM_SCENE);
	if (point >= 0)
		_uniforms[UNIFORM_SCENE]->bind(point);
	std::string light;
	for (int i = 0; i < _lights.size(); ++i) {
		light = string_format(UNIFORM_LIGHTS "[%d]", i);
		point = shader->getVar(light);
		if (point >= 0)
			_uniforms[light]->bind(point);
	}

	glBindVertexArray(cmd.vao);
	if (cmd.inds > 0) {
		if (cmd.ins)
			glDrawElementsInstanced(GL_TRIANGLES, cmd.inds, GL_UNSIGNED_INT, 0, cmd.ins);
		else
			glDrawElements(GL_TRIANGLES, cmd.inds, GL_UNSIGNED_INT, 0);
	}
	else {
		if (cmd.ins)
			glDrawArraysInstanced(GL_TRIANGLES, 0, cmd.verts, cmd.ins);
		else
			glDrawArrays(GL_TRIANGLES, 0, cmd.verts);
	}

	glBindVertexArray(0);
}

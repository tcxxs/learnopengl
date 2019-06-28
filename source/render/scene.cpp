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
	if (!conf["shaders"].IsNull()) {
		for (auto& it: conf["shaders"]) {
			Shader::ptr shader = ShaderMgr::inst().req(it.as<std::string>());
			if (!shader)
				return false;
			pass.shaders.insert(shader);
		}
	}
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
	for (const auto& it: conf["states"]) {
		const std::string& key = it.first.Scalar();
		if (key == "clear") {
			GLbitfield flags = 0;
			for (const auto& ita: it.second) {
				const std::string& arg = ita.Scalar();
				if (arg == "color")
					flags |= GL_COLOR_BUFFER_BIT;
				else if (arg == "depth")
					flags |= GL_DEPTH_BUFFER_BIT;
				else if (arg == "stencil")
					flags |= GL_STENCIL_BUFFER_BIT;
			}

			pass.states.emplace_back([flags] {
				if (flags & GL_COLOR_BUFFER_BIT) {
					glClearColor(BG_COLOR);
				}
				glClear(flags);
			});
		}
		else if (key == "depth") {
			bool enable = it.second[0].as<bool>();
			GLenum flag = GL_LESS;
			const std::string& arg = it.second[1].as<std::string>();
			if (arg == "less")
				flag = GL_LESS;
			else if (arg == "lesseq")
				flag = GL_LEQUAL;

			pass.states.emplace_back([enable, flag] {
				enable ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
				glDepthFunc(flag);
			});
		}
	}

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

void Scene::draw() {
	for (const auto& it: _pass) {
		glViewport(0, 0, WIDTH, HEIGHT);
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_BLEND);
		glEnable(GL_CULL_FACE);

		if (it.out) {
			it.out->useBegin();
		}
		for (const auto& st: it.states)
			st();

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
	UniformInst::ptr& matvp = _uniforms[UNIFORM_MATVP];
	matvp->setVar("view", _cam->getView());
	matvp->setVar("proj", _cam->getProj());

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
	scene->setVar("camera", _cam->getPos());
	scene->setVar("lights", (int)_lights.size());

	CommandQueue cmds;
	for (auto& it: _models) {
		it->draw(cmds);
	}

	for (auto& it: cmds) {
		if (pass.shaders.empty() || pass.shaders.count(it.material->getShader()) > 0)
			drawCommand(it);
	}
}

void Scene::drawCommand(const Command& cmd) {
	cmd.material->use();
	const Shader::ptr& shader = cmd.material->getShader();
	shader->setVars(cmd.attrs);
	glUniformMatrix4fv(shader->getVar("model"), 1, GL_FALSE, glm::value_ptr(cmd.model));

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
	if (cmd.ibosize > 0)
		glDrawElements(GL_TRIANGLES, cmd.ibosize, GL_UNSIGNED_INT, 0);
	else
		glDrawArrays(GL_TRIANGLES, 0, cmd.arrsize);
	glBindVertexArray(0);
}

void Scene::drawPost(const Pass& pass) {
	pass.post->draw(pass.ins);
}

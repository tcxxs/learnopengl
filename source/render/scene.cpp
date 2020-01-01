#include <random>
#include "scene.hpp"
#include "system.hpp"
#include "render/command.hpp"

std::vector<std::string> Scene::list() {
	std::vector<std::string> files;
	std::filesystem::path path = std::filesystem::current_path() / "resource" / "scene";
	for (const auto& it: std::filesystem::directory_iterator(path)) {
		if (!it.is_regular_file())
			continue;

		files.emplace_back(it.path().stem().string());
	}

	return files;
}

void Scene::unload() {
	Config::gen = nullptr;
	if (current) {
		current = nullptr;
	}

	SceneMgr::inst().clear();
	ModelProtoMgr::inst().clear();
	LightProtoMgr::inst().clear();
	PostMgr::inst().clear();
	MaterialMgr::inst().clear();
	ShaderMgr::inst().clear();
	TextureMgr::inst().clear();
	UniformProtoMgr::inst().clear();
	VertexProtoMgr::inst().purge();
}

bool Scene::reload(const std::string& name) {
	unload();
	const Scene::ptr& scene = create(name);
	if (!scene)
		return false;
	scene->active();
	return true;
}

Scene::ptr Scene::create(const std::string& name) {
	Scene::ptr scene = std::make_shared<Scene>();
	scene->setName(name);

	Config conf;
	std::filesystem::path path = std::filesystem::current_path() / "resource" / "scene" / (name + ".yml");
	if (!conf.load(path)) {
		ERR("scene config error, %s", path.c_str());
		return {};
	}

	// TODO: 这个时机不算太好
	Config::gen = std::bind(&Scene::generateConf, scene.get(), std::placeholders::_1);
	scene->_cfuncs.emplace("base", std::bind(&Scene::_genBase, scene.get(), std::placeholders::_1));
	scene->_cfuncs.emplace("camera", std::bind(&Scene::_genCamera, scene.get(), std::placeholders::_1));
	scene->_cfuncs.emplace("light", std::bind(&Scene::_genLight, scene.get(), std::placeholders::_1));
	scene->_cfuncs.emplace("frame", std::bind(&Scene::_genFrame, scene.get(), std::placeholders::_1));
	scene->_cfuncs.emplace("ssao", std::bind(&Scene::_genSSAO, scene.get(), std::placeholders::_1));

	if (!scene->_initFrame(conf["frames"]))
		return {};

	for (const auto& it: conf["cameras"]) {
		const std::string& name = it.first.as<std::string>();
		scene->addCamera(name, it.second);
	}
	const std::string cam = conf["camera"].as<std::string>();
	const auto& find = scene->_cams.find(cam);
	if (find == scene->_cams.end()) {
		ERR("scene camera not found, %s", cam.c_str());
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

	if (!scene->addPass(conf["pass"]))
		return {};

	// TODO: 因为uniform需要shader的布局信息，所以需要shader加载之后
	// 1，用stdlaout自己算；2，用一个特殊shader触发
	if (!scene->_initUniform(UNIFORM_MATVP))
		return {};
	if (!scene->_initUniform(UNIFORM_SCENE))
		return {};
	if (!scene->_initUniform(UNIFORM_LIGHTS, LIGHT_MAX))
		return {};

	return scene;
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

// TODO: 这里只是一个简单的pass generator
bool Scene::addPass(const Config::node& conf) {
	if (!Config::valid(conf))
		return true;

	std::map<std::string, int> names;
	for (int i = 0; i < conf.size(); ++i) {
		const Config::node& it = conf[i];
		Pass::ptr pass;
		if (Config::valid(it["copy"])) {
			const auto& find = names.find(it["copy"].as<std::string>());
			if (find == names.end()) {
				ERR("pass %d, copy not found\n", i);
				return false;
			}

			const Config::node& copy = conf[find->second];
			pass = Pass::create(copy);
		}
		else {
			pass = Pass::create(it);
			names[pass->getName()] = i;
		}

		if (!pass)
			return false;
		_pass.push_back(pass);
	}

	return true;
}

bool Scene::_initUniform(const std::string& name, int count) {
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

bool Scene::_initFrame(const Config::node& conf) {
	if (!Config::valid(conf))
		return true;

	for (const auto& it: conf) {
		const std::string& name = it.first.as<std::string>();
		Frame::ptr frame = Frame::create(it.second);
		if (!frame)
			return false;
		_frames[name] = frame;
	}

	return true;
}

void Scene::draw() {
	int width = SystemMgr::inst().getWidth();
	int height = SystemMgr::inst().getHeight();
	CommandQueue cmds;
	for (const auto& it: _pass) {
		if (!it->getRun())
			continue;

		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 0, string_format("pass %s", it->getName().c_str()).c_str());
		cmds.clear();
		it->drawBegin();

		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 0, "uniforms");
		drawUniforms(it);
		glPopDebugGroup();

		// TODO: 同shader、同attr应该合批
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 0, "collect");
		int n = it->drawPass(cmds, _models);
		glPopDebugGroup();

		if (n >= 0) {
			glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, 0, "draw");
			for (auto& itc: cmds) {
				drawCommand(itc);
			}
			glPopDebugGroup();
		}

		it->drawEnd();
		glPopDebugGroup();
	}
}

// TODO: 其实大部分同一帧都不变
void Scene::drawUniforms(const Pass::ptr& pass) {
	Camera::ptr cam = pass->getCamera();
	if (!cam)
		cam = _cam;

	UniformInst::ptr& matvp = _uniforms[UNIFORM_MATVP];
	matvp->setVar("view", cam->getView());
	matvp->setVar("proj", cam->getProj());

	// TODO: 支持多光源shadow
	float dark = 5.0f / 256.0f;
	for (int i = 0; i < _lights.size(); ++i) {
		UniformInst::ptr& uniform = _uniforms[string_format(UNIFORM_LIGHTS "[%d]", i)];
		const LightInst::ptr& light = _lights[i];
		const LightProto::ptr& proto = light->prototype();
		const LightProto::lighttype type = proto->getType();

		//GLfloat cmax{0.0f}, radius{0.0f};
		glm::vec3 color = proto->attrs.getAttr<glm::vec3>("color");
		uniform->setVar("light.type", (int)type);
		switch (type) {
		case LightProto::LIGHT_DIR:
			uniform->setVar("light.dir", light->getDir());
			uniform->setVar("light.color", color);
			break;
		case LightProto::LIGHT_POINT:
			uniform->setVar("light.pos", light->getPos());
			uniform->setVar("light.color", color);

			//cmax = std::fmaxf(std::fmaxf(diffuse.r, diffuse.g), diffuse.b);
			//radius = (-linear + std::sqrtf(linear * linear - 4 * quadratic * (constant - cmax / dark))) / (2 * quadratic);
			//uniform->setVar("light.radius", quadratic);
			break;
		case LightProto::LIGHT_SPOT:
			uniform->setVar("light.pos", light->getPos());
			uniform->setVar("light.dir", light->getDir());
			uniform->setVar("light.color", color);
			uniform->setVar("light.inner", cos(glm::radians(proto->attrs.getAttr<float>("inner"))));
			uniform->setVar("light.outter", cos(glm::radians(proto->attrs.getAttr<float>("outter"))));

			//cmax = std::fmaxf(std::fmaxf(diffuse.r, diffuse.g), diffuse.b);
			//radius = (-linear + std::sqrtf(linear * linear - 4 * quadratic * (constant - cmax / dark))) / (2 * quadratic);
			//uniform->setVar("light.radius", quadratic);
			break;
		}
	}

	UniformInst::ptr& scene = _uniforms[UNIFORM_SCENE];
	scene->setVar("camera", cam->getPos());
	scene->setVar("lights", (int)_lights.size());
}

void Scene::drawCommand(const Command& cmd) {
	if (cmd.buffs.empty()) {
		// draw backbuffer or color0
	}
	else {
		if (cmd.buffs.size() == 1 && cmd.buffs[0] == GL_NONE)
			// draw none
			return;
		else
			// draw buffers
			glDrawBuffers((GLsizei)cmd.buffs.size(), cmd.buffs.data());
	}

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

// TODO: 现在是静态的，或许应该要支持动态
std::any Scene::generateConf(const Config::node& conf) {
	const auto& find = _cfuncs.find(conf[0].as<std::string>());
	if (find == _cfuncs.end())
		return {};

	return find->second(conf);
}

std::any Scene::_genBase(const Config::node& conf) {
	if (conf.size() < 2)
		return {};

	const std::string& type = conf[1].as<std::string>();
	if (type == "cubevp") {
		if (conf.size() < 3)
			return {};

		const std::any ret = Config::guess(conf[2]);
		if (!ret.has_value())
			return {};
		if (ret.type() != typeid(glm::vec3))
			return {};

		const glm::vec3 pos = std::any_cast<glm::vec3>(ret);
		std::vector<glm::mat4> mats;
		glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, PROJ_NEAR, PROJ_FAR);
		mats.push_back(proj * glm::lookAt(pos, pos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
		mats.push_back(proj * glm::lookAt(pos, pos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
		mats.push_back(proj * glm::lookAt(pos, pos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
		mats.push_back(proj * glm::lookAt(pos, pos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
		mats.push_back(proj * glm::lookAt(pos, pos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
		mats.push_back(proj * glm::lookAt(pos, pos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));
		return mats;
	}

	return {};
}

std::any Scene::_genCamera(const Config::node& conf) {
	if (conf.size() < 3)
		return {};

	const std::string& name = conf[1].as<std::string>();
	const std::string& type = conf[2].as<std::string>();
	if (type == "light") {
		for (const auto& it: _lights) {
			if (it->getName() != name)
				continue;
			if (it->prototype()->getType() != LightProto::LIGHT_SPOT)
				continue;

			const glm::vec3 pos = it->getPos();
			const glm::vec3 dir = it->getDir();
			Camera::ptr cam = Camera::create();
			if (!cam)
				return {};
			cam->setFov(_cam->getFov());
			cam->lookAt(pos, pos + dir);
			return cam;
		}
	}

	return {};
}

std::any Scene::_genLight(const Config::node& conf) {
	if (conf.size() < 3)
		return {};

	const std::string& name = conf[1].as<std::string>();
	const std::string& type = conf[2].as<std::string>();
	for (int i = 0; i < _lights.size(); ++i) {
		const LightInst::ptr light = _lights[i];
		if (light->getName() != name)
			continue;

		const glm::vec3 pos = light->getPos();
		const glm::vec3 dir = light->getDir();
		if (type == "index") {
			return i;
		}
		else if (type == "pos") {
			return pos;
		}
		else if (type == "vp") {
			const float aspect = (float)SystemMgr::inst().getWidth() / (float)SystemMgr::inst().getHeight();
			glm::mat4 proj = glm::perspective(glm::radians(_cam->getFov()), aspect, PROJ_NEAR, PROJ_FAR);
			glm::mat4 view = glm::lookAt(pos, pos + dir, Camera::up);
			return proj * view;
		}
	}

	return {};
}

std::any Scene::_genFrame(const Config::node& conf) {
	if (conf.size() < 2)
		return {};

	const std::string& name = conf[1].as<std::string>();
	const auto& find = _frames.find(name);
	if (find == _frames.end())
		return {};

	if (conf.size() > 2) {
		const std::string& attach = conf[2].as<std::string>();
		return std::make_pair(find->second, attach);
	}
	else
		return find->second;
}

std::any Scene::_genSSAO(const Config::node& conf) {
	if (conf.size() < 2)
		return {};

	static std::random_device rand_d;
	static std::uniform_real_distribution<GLfloat> rand_float(0.0, 1.0);
	static std::default_random_engine rand_gen(rand_d());

	const std::string& name = conf[1].as<std::string>();
	int num = conf[2].as<int>();
	if (name == "samples") {

		GLfloat scale;
		std::vector<glm::vec3> kernel;
		for (int i = 0; i < num; ++i) {
			glm::vec3 sample{rand_float(rand_gen) * 2.0 - 1.0,
			                 rand_float(rand_gen) * 2.0 - 1.0,
			                 rand_float(rand_gen)};
			scale = i / float(num);
			scale = 0.1f + (1.0f - 0.1f) * scale * scale;
			sample = glm::normalize(sample) * rand_float(rand_gen) * scale;
			kernel.push_back(sample);
		}

		return {kernel};
	}
	else if (name == "noise") {
		static GLuint tex{0};
		if (!tex) {
			std::vector<glm::vec3> noise;
			for (int i = 0; i < num * num; i++) {
				glm::vec3 vec{rand_float(rand_gen) * 2.0 - 1.0,
				              rand_float(rand_gen) * 2.0 - 1.0,
				              rand_float(rand_gen) * 2.0 - 1.0};
				noise.push_back(vec);
			}

			glGenTextures(1, &tex);
			glBindTexture(GL_TEXTURE_2D, tex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, num, num, 0, GL_RGB, GL_FLOAT, &noise[0]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}

		return Texture::val(tex);
	}

	return {};
}

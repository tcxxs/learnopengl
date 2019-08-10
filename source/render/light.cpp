#include "light.hpp"

LightProto::ptr LightProto::create(const std::string& name) {
	if (_confs.root().IsNull()) {
		std::filesystem::path path = std::filesystem::current_path() / "resource" / "lights.yml";
		if (!_confs.load(path)) {
			std::cout << "lights config error";
			return {};
		}
	}
	Config::node conf = _confs[name];
	if (!Config::valid(conf)) {
		std::printf("lights config not find, %s", name.c_str());
		return {};
	}

	LightProto::ptr proto = std::make_shared<LightProto>();
	proto->setName(name);

	if (!proto->attrs.updateConf(conf)) {
		return {};
	}

	const std::string& type = conf["type"].as<std::string>();
	if (type == "dir")
		proto->_type = LIGHT_DIR;
	else if (type == "point")
		proto->_type = LIGHT_POINT;
	else if (type == "spot")
		proto->_type = LIGHT_SPOT;
	else
		return {};

	return proto;
}

LightInst::ptr LightInst::create(const proto_ptr& proto, const Config::node& conf) {
	LightInst::ptr light = std::make_shared<LightInst>();

	if (Config::valid(conf["name"]))
		light->setName(conf["name"].as<std::string>());
	Config::node pos = conf["pos"];
	if (pos.IsDefined()) {
		light->setPos(pos.as<glm::vec3>());
	}
	Config::node dir = conf["dir"];
	if (dir.IsDefined()) {
		light->setDir(dir.as<glm::vec3>());
	}
	return light;
}
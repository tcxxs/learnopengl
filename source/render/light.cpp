#include "light.hpp"

LightProto::ptr LightProto::create(const std::string& name) {
	LightProto::ptr proto = std::shared_ptr<LightProto>(new LightProto());
	proto->setName(name);

	std::filesystem::path path = std::filesystem::current_path() / "resource" / "light" / (name + ".yml");
	if (!proto->_conf.load(path)) {
		std::cout << "proto config error, " << path << std::endl;
		return {};
	}

	if (!proto->attrs.updateConf(proto->_conf.root())) {
		return {};
	}

	return proto;
}

LightInst::ptr LightInst::create(const proto_ptr& proto, const Config::node& conf) {
	LightInst::ptr light = std::shared_ptr<LightInst>(new LightInst());
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
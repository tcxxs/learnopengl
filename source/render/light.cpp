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
	if (!conf.IsDefined())
		return {};

	LightProto::ptr proto = std::shared_ptr<LightProto>(new LightProto());
	proto->setName(name);

	if (!proto->attrs.updateConf(conf)) {
		return {};
	}
	if (proto->attrs.hasAttr("inner"))
		proto->_type = LIGHT_SPOT;
	else if (proto->attrs.hasAttr("constant"))
		proto->_type = LIGHT_POINT;
	else
		proto->_type = LIGHT_DIR;

	return proto;
}

LightInst::ptr LightInst::create(const proto_ptr& proto, const Config::node& conf) {
	LightInst::ptr light = std::shared_ptr<LightInst>(new LightInst());

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
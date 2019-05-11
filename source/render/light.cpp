#include "light.hpp"

Light::ptr Light::create(const std::string& name) {
	Light::ptr light = std::shared_ptr<Light>(new Light());
	std::filesystem::path path = std::filesystem::current_path() / "resource" / "light" / (name + ".yml");
	if (!light->_conf.load(path)) {
		std::cout << "light config error, " << path << std::endl;
		return {};
	}

	for (const auto& it: light->_conf.root()) {
		const std::string& key = it.first.as<std::string>();
		std::any value = Config::guess(it.second);
		if (!value.has_value()) {
			std::cout << "light attribute guess fail, " << key << std::endl;
			continue;
		}
		light->attrs.setAttr(key, value);
	}

	return light;
}

Light::~Light() {
}

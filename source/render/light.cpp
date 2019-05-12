#include "light.hpp"

Light::ptr Light::create(const std::string& name) {
	Light::ptr light = std::shared_ptr<Light>(new Light());
	std::filesystem::path path = std::filesystem::current_path() / "resource" / "light" / (name + ".yml");
	if (!light->_conf.load(path)) {
		std::cout << "light config error, " << path << std::endl;
		return {};
	}

	if (!light->attrs.guessAttrs(light->_conf.root())) {
		return {};
	}

	return light;
}

Light::~Light() {
}

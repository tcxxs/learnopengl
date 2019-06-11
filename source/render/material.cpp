#include "material.hpp"
#include "render/shader.hpp"

Material::ptr Material::create(const std::string& name, const Config::node& conf) {
	Material::ptr mate = std::shared_ptr<Material>(new Material());
	mate->setName(name);

	mate->_shader = ShaderMgr::inst().req(conf["shader"].as<std::string>());
	if (!mate->_shader) {
		std::cout << "material shader error, " << name << std::endl;
		return {};
	}

	const auto node = conf["vars"];
	if (node.IsDefined()) {
		if (!mate->attrs.updateConf(node))
			return {};
		for (auto& it : mate->attrs) {
			if (it.second.type() == typeid(std::string)) {
				const auto& tex = TextureMgr::inst().req(std::any_cast<std::string&>(it.second));
				if (!tex)
					return {};
				it.second = tex;
			}
		}
	}

	return mate;
}

bool Material::use() {
	_shader->use();
	_shader->setVars(attrs);
	return true;
}
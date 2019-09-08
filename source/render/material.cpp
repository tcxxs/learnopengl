#include "material.hpp"
#include "render/shader.hpp"

Material::ptr Material::create(const std::string& name) {
	if (_confs.root().IsNull()) {
		std::filesystem::path path = std::filesystem::current_path() / "resource" / "materials.yml";
		if (!_confs.load(path)) {
			ERR("materials config error");
			return {};
		}
	}
	Config::node conf = _confs[name];
	if (!Config::valid(conf)) {
		ERR("materials config not find, %s", name.c_str());
		return {};
	}
	
	Material::ptr mate = std::make_shared<Material>();
	mate->setName(name);

	mate->_shader = ShaderMgr::inst().req(conf["shader"].as<std::string>());
	if (!mate->_shader) {
		ERR("material shader error, %s", name.c_str());
		return {};
	}

	const auto node = conf["vars"];
	if (Config::valid(node)) {
		if (!mate->attrs.updateConf(node))
			return {};
		for (auto& it : mate->attrs) {
			if (it.second.type() == typeid(std::string)) {
				const auto& tex = TextureMgr::inst().req(std::any_cast<std::string&>(it.second));
				if (!tex)
					return {};
				it.second = tex->getValue();
			}
			else if (it.second.type() == typeid(strcube)) {
				const auto& tex = TextureMgr::inst().create(std::any_cast<strcube&>(it.second));
				if (!tex)
					return {};
				it.second = tex->getValue();
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
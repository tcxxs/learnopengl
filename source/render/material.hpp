#pragma once

#include "glad/glad.h"
#include "config.hpp"
#include "utils/resource.hpp"
#include "utils/utils.hpp"
#include "render/shader.hpp"

class Material : public Res<Material> {
public:
	static ptr create(const std::string& name);

	bool use();
	inline const Shader::ptr& getShader() const { return _shader; }

public:
	Attributes attrs;
private:
	inline static Config _confs;
	Shader::ptr _shader;
};

using MaterialMgr = ResMgr<Material>;

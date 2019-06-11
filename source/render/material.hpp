#pragma once

#include "glad/glad.h"
#include "config.hpp"
#include "utils/resource.hpp"
#include "utils/utils.hpp"
#include "render/shader.hpp"

class Material : public Res<Material> {
public:
	static ptr create(const std::string& name, const Config::node& conf);

	bool use();
	inline const Shader::ptr& getShader() const { return _shader; }

public:
	Attributes attrs;
private:
	Shader::ptr _shader;
};

using MaterialMgr = ResMgr<Material>;

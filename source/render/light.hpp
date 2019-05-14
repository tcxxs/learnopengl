#pragma once

#include <any>
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "glad/glad.h"
#include "utils/utils.hpp"
#include "utils/resource.hpp"

class Light: public Res<Light> {
public:
	static ptr create(const std::string& name);
	virtual ~Light();

	inline void setPos(const glm::vec3& pos) { _pos = pos; }
	inline const glm::vec3& getPos() const { return _pos; }

public:
	Attributes attrs;
private:
	Config _conf;
	glm::vec3 _pos{0.0f};
};

using LightMgr = ResMgr<Light>;

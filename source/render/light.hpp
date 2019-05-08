#pragma once

#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "glad/glad.h"
#include "utils/resource.hpp"

class Light: public Res<Light> {
public:
	static ptr create();
	virtual ~Light();

	inline void setPos(const glm::vec3& pos) { _pos = pos; }
	inline const glm::vec3& getPos() const { return _pos; }
	inline void setColor(const glm::vec3& color) { _color = color; }
	inline const glm::vec3& getColor() const { return _color; }

private:
	glm::vec3 _pos{0.0f};
	glm::vec3 _color{1.0f};
};

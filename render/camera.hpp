#pragma once

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "utils/resource.hpp"

class Camera: public Res<Camera> {
public:
	static ptr create();
	virtual ~Camera();

	inline void setPos(const glm::vec3& pos) { 
		_pos = pos;
		updateMatrix();
	}
	inline const glm::vec3& getPos() const { return _pos; }

	inline void setDir(const glm::vec3& dir) { 
		_dir = dir;
		updateMatrix();
	}
	inline const glm::vec3& getDir() const { return _dir; }

	inline void lookAt(const glm::vec3& pos, const glm::vec3& target) {
		_mat = glm::lookAt(pos, target, Camera::_upy);
		_pos = pos;
		_dir = target - pos;
	}
	inline const glm::mat4& getMatrix() const { return _mat; }
	inline glm::vec3 getFront() const { return glm::normalize(-_dir); }
	inline glm::vec3 getRight() const { return glm::normalize(glm::cross(Camera::_upy, getFront())); }
	inline glm::vec3 getUp() const { return glm::normalize(glm::cross(getFront(), getRight())); }

private:
	inline void updateMatrix() {
		_mat = glm::lookAt(_pos, _pos + _dir, Camera::_upy);
	}

private:
	inline static const glm::vec3 _upy{0.0f, 1.0f, 0.0f};
	glm::vec3 _pos{0.0f}, _dir{0.0f};
	glm::mat4 _mat{1.0f};
};

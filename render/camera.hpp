#pragma once

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "config.hpp"
#include "utils/resource.hpp"

class Camera : public Res<Camera> {
public:
	static ptr create();
	virtual ~Camera();

	inline void lookAt(const glm::vec3& pos, const glm::vec3& tar) {
		_pos = pos;
		_front = glm::normalize(pos - tar);
		_lookatToCoord();
		_coordToEuler();
		_lookatToMatrix();
	}
	inline void setPos(const glm::vec3& pos) {
		_pos = pos;
		_lookatToCoord();
		_coordToEuler();
		_lookatToMatrix();
	}
	inline void setTarget(const glm::vec3& tar) {
		_front = glm::normalize(_pos - tar);
		_lookatToCoord();
		_coordToEuler();
		_lookatToMatrix();
	}
	inline void setYaw(float yaw) {
		_yaw = fmod(yaw, 2 * PI);
		_eulerToLookat();
		_lookatToCoord();
		_lookatToMatrix();
	}
	inline void setPitch(float pitch) {
		_pitch = fmod(pitch, 2 * PI);
		_eulerToLookat();
		_lookatToCoord();
		_lookatToMatrix();
	}

	inline const glm::mat4& getMatrix() const { return _mat; }
	inline const glm::vec3& getPos() const { return _pos; }
	inline const glm::vec3& getFront() const { return _front; }
	inline const glm::vec3& getRight() const { return _right; }
	inline const glm::vec3& getUp() const { return _up; }
	inline float getPitch() const { return _pitch; }
	inline float getYaw() const { return _yaw; }

private:
	inline void _lookatToMatrix() { 
		_mat = glm::lookAt(_pos, _pos - _front, Camera::_upy);
	}
	inline void _lookatToCoord() {
		_right = glm::normalize(glm::cross(Camera::_upy, _front));
		_up = glm::normalize(glm::cross(_front, _right));
	}
	inline void _coordToEuler() {
		_yaw = atan(_front.z / _front.x);
		if (_yaw < 0) {
			_yaw = PI - fmod(-_yaw, PI);
		}
		else {
			_yaw = fmod(_yaw, PI);
		}
		_pitch = fmod(asin(_front.y / glm::length(_front)), 2 * PI);
	}
	inline void _eulerToLookat() {
		_front.x = cos(_yaw) * cos(_pitch);
		_front.y = sin(_pitch);
		_front.z = sin(_yaw) * cos(_pitch);
		_front = glm::normalize(_front);
	}

private:
	inline static const glm::vec3 _upy{0.0f, 1.0f, 0.0f};
	glm::mat4 _mat{1.0f};
	glm::vec3 _pos{0.0f};
	glm::vec3 _front{0.0f}, _right{0.0f}, _up{0.0f};
	float _yaw{0.0f}, _pitch{0.0f}, _roll{0.0f};
};

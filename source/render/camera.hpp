#pragma once

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "glm/gtx/string_cast.hpp"
#include "config.hpp"
#include "event.hpp"
#include "utils/resource.hpp"
#include "utils/utils.hpp"

// TODO: 由于up一直是+y轴，所以镜头往下空翻的时候，会有点奇怪
class Camera: public Res<Camera> {
public:
	static ptr create();
	virtual ~Camera();

	inline void lookAt(const glm::vec3& pos, const glm::vec3& tar) {
		_pos = pos;
		_front = glm::normalize(pos - tar);
		_lookatToCoord();
		_coordToEuler();
		_lookatToView();
	}
	inline void setPos(const glm::vec3& pos) {
		_pos = pos;
		_lookatToCoord();
		_coordToEuler();
		_lookatToView();
	}
	inline void setTarget(const glm::vec3& tar) {
		_front = glm::normalize(_pos - tar);
		_lookatToCoord();
		_coordToEuler();
		_lookatToView();
	}
	inline void setYaw(float yaw) {
		if (yaw < 0)
			_yaw = 2 * PI - fmod(-yaw, 2 * PI);
		else
			_yaw = fmod(yaw, 2 * PI);
		_eulerToLookat();
		_lookatToCoord();
		_lookatToView();
	}
	inline void setPitch(float pitch) {
		_pitch = fmod(pitch, 2 * PI);
		_eulerToLookat();
		_lookatToCoord();
		_lookatToView();
	}

	inline const glm::mat4& getView() const { return _view; }
	inline const glm::vec3& getPos() const { return _pos; }
	inline const glm::vec3& getFront() const { return _front; }
	inline const glm::vec3& getRight() const { return _right; }
	inline const glm::vec3& getUp() const { return _up; }
	inline float getPitch() const { return _pitch; }
	inline float getYaw() const { return _yaw; }

	inline void setFov(float fov) {
		_fov = fov;
		_proj = glm::perspective(glm::radians(_fov), (float)EventMgr::inst().getWidth() / (float)EventMgr::inst().getHeight(), PROJ_NEAR, PROJ_FAR);
	}

	inline const glm::mat4& getProj() const { return _proj; }
	inline float getFov() const { return _fov; }

private:
	inline void _lookatToView() {
		_view = glm::lookAt(_pos, _pos - _front, Camera::up);
	}
	inline void _lookatToCoord() {
		_right = glm::normalize(glm::cross(Camera::up, _front));
		_up = glm::normalize(glm::cross(_front, _right));
	}
	inline void _coordToEuler() {
		_yaw = atan(_front.z / _front.x);
		if (_front.x < 0)
			_yaw += PI;
		else if (_front.z < 0)
			_yaw += 2 * PI;
		_yaw = fmod(_yaw, 2 * PI);
		_pitch = fmod(asin(_front.y / glm::length(_front)), 2 * PI);

		//std::cout << glm::to_string(_front) << "->" << _yaw << std::endl;
	}
	inline void _eulerToLookat() {
		_front.x = cos(_yaw) * cos(_pitch);
		_front.y = sin(_pitch);
		_front.z = sin(_yaw) * cos(_pitch);
		_front = glm::normalize(_front);

		Logger::debug("camera", "%.2fyaw, %.2fpitch -> %s", _yaw, _pitch, glm::to_string(_front).c_str());
	}

public:
	inline static const glm::vec3 up{0.0f, 1.0f, 0.0f};

private:
	glm::mat4 _view{1.0f};
	glm::vec3 _pos{0.0f};
	glm::vec3 _front{0.0f}, _right{0.0f}, _up{0.0f};
	float _yaw{0.0f}, _pitch{0.0f}, _roll{0.0f};

	glm::mat4 _proj{1.0f};
	float _fov{0};
};

#include "camera.hpp"

Camera::ptr Camera::create() {
	Camera::ptr camera = std::shared_ptr<Camera>(new Camera());
	
	return std::move(camera);
}

Camera::~Camera() {
}

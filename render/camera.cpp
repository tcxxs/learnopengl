#include "camera.hpp"

Camera::ptr Camera::create() {
	Camera::ptr camera = std::shared_ptr<Camera>(new Camera());
	
	return camera;
}

Camera::~Camera() {
}

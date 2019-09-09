#include "camera.hpp"

Camera::ptr Camera::create() {
	Camera::ptr camera = std::make_shared<Camera>();
	
	return camera;
}

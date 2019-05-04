#include "light.hpp"

Light::ptr Light::create() {
	Light::ptr light = std::shared_ptr<Light>(new Light());

	return light;
}

Light::~Light() {
}

#pragma once

#include <string>
#include <filesystem>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "config.hpp"
#include "event.hpp"
#include "utils/resource.hpp"
#include "render/model.hpp"
#include "render/shader.hpp"

void oglFeature();
void onRender();
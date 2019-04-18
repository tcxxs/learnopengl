#pragma once

#include <iostream>
#include "GLFW/glfw3.h"
#include "config.hpp"

void onResize(GLFWwindow* window, int width, int height);
void onInput(GLFWwindow* window);
bool oglError();
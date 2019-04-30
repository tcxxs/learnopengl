#pragma once

#include <iostream>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "utils/pattern.hpp"
#include "render/render.hpp"
#include "config.hpp"

void onResize(GLFWwindow* window, int width, int height);

class Event: public NoCopy {
public:
    void init(GLFWwindow* window, int fps);

    void onUpdate();
    void onInput();


private:
    GLFWwindow* _window;
    float _time_last{0.0f}, _time_delta{0.0f};
    float _frame_interv{0.0f}, _frame_last{0.0f}, _frame_delta{0.0f};
};

using EventMgr = Singleton<Event>;
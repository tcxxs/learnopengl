#pragma once

#include <iostream>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/gtx/string_cast.hpp"
#include "utils/pattern.hpp"
#include "render/render.hpp"
#include "config.hpp"

class Event: public NoCopy {
public:
	~Event();

	bool init();
	void process();

	void onResize(int width, int height);
	void onInput();
	void onMouse(float xpos, float ypos);
	void onScroll(float xoffset, float yoffset);

	inline int getFPS() const { return _fps; }
	inline int getWidth() const { return _width; }
	inline int getHeight() const { return _height; }
	inline int getMSAA() const { return _msaa; }
	inline const glm::vec3& getBGColor() const { return _bgcolor; }

private:
	bool _initConfig();
	bool _initWindow();

private:
	int _fps{0};
	int _width{0}, _height{0};
	glm::vec3 _bgcolor{0.0f};
	int _msaa{0};
	GLFWwindow* _window{nullptr};
	float _time_last{0.0f}, _time_delta{0.0f};
	float _frame_interv{0.0f}, _frame_last{0.0f}, _frame_delta{0.0f};
	bool _mouse_init{false};
	float _mouse_x{0.0f}, _mouse_y{0.0f};
	float _fox{0.0f};
};

using EventMgr = Singleton<Event>;
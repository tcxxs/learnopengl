#include "ui/ui.hpp"

#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

bool UI::init(GLFWwindow* window) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 430 core");

	_fonts["default"] = io.Fonts->AddFontDefault();
	if (!_fonts["default"]) {
		std::printf("ui init font error, default");
		return false;
	}
	_fonts["han"] = io.Fonts->AddFontFromFileTTF("resource\\font\\SourceHanSansCN-Regular.otf", 16.0f, NULL, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
	if (!_fonts["han"]) {
		std::printf("ui init font error, han");
		return false;
	}
	_fonts["yahei"] = io.Fonts->AddFontFromFileTTF("resource\\font\\YaHei.Consolas.1.12.ttf", 16.0f, NULL, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
	if (!_fonts["yahei"]) {
		std::printf("ui init font error, yahei");
		return false;
	}

	return true;
}

bool UI::render() {
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");          // Display some text (you can use a format strings too)

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f

		if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	return true;
}

UI::~UI() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

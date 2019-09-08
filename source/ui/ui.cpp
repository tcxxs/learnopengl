#include "ui/ui.hpp"

#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

bool UI::init(GLFWwindow* window) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;
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
	_fonts["han"] = io.Fonts->AddFontFromFileTTF("resource\\font\\SourceHanSansCN-Regular.otf", 24.0f, NULL, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
	if (!_fonts["han"]) {
		std::printf("ui init font error, han");
		return false;
	}
	_fonts["yahei"] = io.Fonts->AddFontFromFileTTF("resource\\font\\YaHei.Consolas.1.12.ttf", 24.0f, NULL, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
	if (!_fonts["yahei"]) {
		std::printf("ui init font error, yahei");
		return false;
	}

	return true;
}

UI::~UI() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

bool UI::onRender()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	static bool _show_log{false};
	ImGui::PushFont(_fonts["han"]);
	ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);
	ImGui::Begin("debug tools");
	ImGui::Checkbox("log", &_show_log);
	if (_show_log) {
		_renderLog();
	}
	ImGui::End();
	ImGui::PopFont();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	return true;
}

bool UI::_renderLog() {
	static bool _auto_scroll{true};

	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
	ImGui::Begin("log");
	
	ImGui::Checkbox("auto scroll", &_auto_scroll);
	ImGui::Separator();

	const std::vector<std::string>& logs = Logger::get();
	ImGui::BeginChild("scrolling", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
	ImGuiListClipper clipper;
	clipper.Begin((int)logs.size());
	while (clipper.Step()) {
		for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
			const std::string& log = logs[i];
			ImGui::TextUnformatted(log.c_str(), log.c_str() + log.size());
		}
	}
	clipper.End();
	if (_auto_scroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
		ImGui::SetScrollHereY(1.0f);

	ImGui::End();
	return true;
}

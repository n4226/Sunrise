#include <srpch.h>
#include "OnboardWorldBGScene.h"


void sunrise::OnboardWorldBGScene::load()
{
	WorldScene::load();

	playerLLA = glm::dvec3(40.68953, -74.02645, 500.0);
}

void TextCentered(const std::string& text) {
	float win_width = ImGui::GetWindowSize().x;
	float text_width = ImGui::CalcTextSize(text.c_str()).x;

	// calculate the indentation that centers the text on one line, relative
	// to window left, regardless of the `ImGuiStyleVar_WindowPadding` value
	float text_indentation = (win_width - text_width) * 0.5f;

	// if text is too long to be drawn on one line, `text_indentation` can
	// become too small or even negative, so we check a minimum indentation
	float min_indentation = 20.0f;
	if (text_indentation <= min_indentation) {
		text_indentation = min_indentation;
	}

	ImGui::SameLine(text_indentation);
	ImGui::PushTextWrapPos(win_width - text_indentation);
	ImGui::TextWrapped(text.c_str());
	ImGui::PopTextWrapPos();
}

void sunrise::OnboardWorldBGScene::onDrawUI()
{

	//creatge an imovabled, not resizable window in the middle of the screen
	
	ImGui::Begin("Onboard World", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

	//set the window size
	ImGui::SetWindowSize(ImVec2(800, 500));
	//center the window in the screen
	ImGui::SetWindowPos(ImVec2(0.5f * (ImGui::GetIO().DisplaySize.x - ImGui::GetWindowSize().x), 0.5f * (ImGui::GetIO().DisplaySize.y - ImGui::GetWindowSize().y)));

	TextCentered("Welcome to Sunrise");
	
	ImGui::Spacing();

	if (ImGui::Button("Start")) {
		//start the game
	}

	ImGui::End();
}

void sunrise::OnboardWorldBGScene::update()
{
	WorldScene::update();
}

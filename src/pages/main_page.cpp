#define IMGUI_DEFINE_MATH_OPERATORS

#include "../menu.hpp"
#include <imgui_ext/animations.hpp>
#include <imgui_ext/helpers.hpp>
#include <imgui/imgui_internal.h>

using namespace ImAnim;
using namespace std::chrono;

enum AVAILABLE_GAMES : int {
	PERMANENT,
	TEMPORARY,
};

const char* get_game_as_str_icon(int game) {
	switch (game) {
	case AVAILABLE_GAMES::PERMANENT:
		return "cs2";
		break;
	case AVAILABLE_GAMES::TEMPORARY:
		return "dota2";
		break;
	}
}

const char* get_game_as_str(int game) {
	switch (game) {
	case AVAILABLE_GAMES::PERMANENT:
		return "Permanent";
		break;
	case AVAILABLE_GAMES::TEMPORARY:
		return "Temporary";
		break;
	}
}

void Menu::main_page() {
	const auto window = ImGui::GetCurrentWindow();
	const auto draw_list = window->DrawList;
	const ImVec2 win_pos = window->Pos;
	const ImVec2 win_size = window->Size;

	static bool spoof_done = false;
	static bool spoof_progress = false;
	static bool spoof_target = false;

    static auto start = std::chrono::steady_clock::now();
	static int elapsed;

	static int current_game = 0;// AVAILABLE_GAMES::CS2;
	
	bool game_cond = (spoof_done || spoof_progress);
	std::string game_anim_name = "GameChange";
	auto game_anim_builder = AnimBuilder<float>().range(1.f, 0.f).easing(Easing::QuadraticEaseInOut).duration(milliseconds(250)).condition(game_cond).play_policy(PlayPolicy::ENDLESS);
	float game_change = game_anim_builder.build(game_anim_name, ImGui::GetIO().DeltaTime);


	if (game_change != 0.f) {
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, game_change * page_alpha);
		ImGui::SetCursorPos(ImVec2((win_size.x - 170.f) * 0.5f, 324.f - 10.f * game_change));
		Combo("Game", current_game, { "Permanent", "Temporary" }, { image("cs2"), image("minecraft") });
		ImGui::PopStyleVar();
	}

	ImGui::SetCursorPos(ImVec2(204, 166));
	if (SpoofButton(ICON_FA_MICROCHIP, ImVec2(128, 128), spoof_done, spoof_target)) {
		spoof_target = !spoof_target;
		spoof_progress = true;
		start = std::chrono::steady_clock::now();
	}

	if (spoof_progress) {
		constexpr auto duration = std::chrono::milliseconds(2500);
		elapsed = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
		if (std::chrono::steady_clock::now() - start > duration) {
			spoof_done = spoof_target;
			spoof_progress = false;
		}
	}


	// Success notify
	bool cond = spoof_done && spoof_target;
	std::string anim_name = "Success";
	auto anim_builder = AnimBuilder<float>().range(0.f, 1.f).easing(Easing::QuadraticEaseInOut).duration(milliseconds(500)).condition(cond).play_policy(PlayPolicy::ENDLESS);
	float success = anim_builder.build(anim_name, ImGui::GetIO().DeltaTime);

	const std::string game_text = local("game").str + get_game_as_str(current_game);
	const std::string success_text = local("success").str + " %d s"s;

	const ImVec2 circle_size = ImGui::CalcTextSize(ICON_FA_CIRCLE_CHECK);
	const ImVec2 success_size = ImGui::CalcTextSize(success_text.c_str());
	const float full_text_width = circle_size.x + 5.f + success_size.x;

	const ImVec2 game_size = ImGui::CalcTextSize(ICON_FA_COMPUTER);
	const ImVec2 game_text_size = ImGui::CalcTextSize(game_text.c_str());
	const float full_game_text_width = game_size.x + 5.f + game_text_size.x;

	ImGui::SetCursorPos(ImVec2((win_size.x - full_text_width) * 0.5f, 345.33f + 20.f * (1.f - success)));
	ImGui::TextColored(color(COLORS::MAIN, success), ICON_FA_CIRCLE_CHECK);

	ImGui::SetCursorPos(ImVec2((win_size.x - full_text_width) * 0.5f + circle_size.x + 5.f, 344.f + 20.f * (1.f - success)));
	ImGui::PushStyleColor(ImGuiCol_Text, color(COLORS::WHITE, success));
	ImGui::Text(success_text.c_str(), elapsed);
	ImGui::PopStyleColor();

	ImGui::SetCursorPos(ImVec2((win_size.x - full_game_text_width) * 0.5f, 96.33f - 20.f * (1.f - success)));
	ImGui::TextColored(color(COLORS::MAIN, success), ICON_FA_COMPUTER);

	ImGui::SetCursorPos(ImVec2((win_size.x - full_game_text_width) * 0.5f + game_size.x + 5.f, 95.f - 20.f * (1.f - success)));
	ImGui::PushStyleColor(ImGuiCol_Text, color(COLORS::WHITE, success));
	ImGui::Text(game_text.c_str());
	ImGui::PopStyleColor();
}
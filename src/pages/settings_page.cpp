#define IMGUI_DEFINE_MATH_OPERATORS

#include "../menu.hpp"
#include <imgui_ext/animations.hpp>
#include <imgui_ext/helpers.hpp>
#include <imgui/imgui_internal.h>

using namespace ImAnim;
using namespace std::chrono;

void Menu::settings_page() {
	const auto window = ImGui::GetCurrentWindow();
	const auto draw_list = window->DrawList;
	const ImVec2 win_pos = window->Pos;
	const ImVec2 win_size = window->Size;
	const auto style = ImGui::GetStyle();

	static bool tmp_active = true, wifi_active = false, usb_active = false;

	ImGui::SetCursorPos(ImVec2(112, 139));
	BeginChild("Options", ImVec2(312, 74), ImGuiChildFlags_Border);
	static bool mac_spoof = false, another_settings = false;
	Checkbox(local("optionsmac"), ICON_FA_WAVEFORM_LINES, mac_spoof);
	Checkbox(local("optionsanother"), ICON_FA_SHAPES, another_settings);
	EndChild();

	ImGui::SetCursorPos(ImVec2(112, 221));
	BeginChild("Status", ImVec2(312, 99));
	ImGui::PushFont(font(FONTS::SFPRO_SMALL_REGULAR));

	// Anims
	const float delta_time = ImGui::GetIO().DeltaTime;

	auto status_anim_builder = AnimBuilder<ImVec4>();
	status_anim_builder.range(color(COLORS::SECONDARY), color(COLORS::MAIN)).easing(Easing::QuadraticEaseInOut).play_policy(PlayPolicy::ENDLESS).duration(milliseconds(350));

	ImVec4 tmp_col = status_anim_builder.condition(tmp_active).build("Status/Tmp/Color", delta_time);
	ImVec4 wifi_col = status_anim_builder.condition(wifi_active).build("Status/Wifi/Color", delta_time);
	ImVec4 usb_col = status_anim_builder.condition(usb_active).build("Status/Usb/Color", delta_time);

	ImGui::SetCursorPos(ImVec2(17.17f, 18.f));
	ImGui::TextColored(tmp_col, ICON_FA_CIRCLE_CHECK);
	ImGui::SetCursorPos(ImVec2(39.5f, 17.2f));
	ImGui::Text(local("statustmp").ch());

	ImGui::SetCursorPos(ImVec2(17.17f, 43.f));
	ImGui::TextColored(wifi_col, ICON_FA_CIRCLE_CHECK);
	ImGui::SetCursorPos(ImVec2(39.5f, 42.2f));
	ImGui::Text(local("statuswifi").ch());

	ImGui::SetCursorPos(ImVec2(17.17f, 68.f));
	ImGui::TextColored(usb_col, ICON_FA_CIRCLE_CHECK);
	ImGui::SetCursorPos(ImVec2(39.5f, 67.2f));
	ImGui::Text(local("statususb").ch());

	ImGui::PopFont();
	EndChild();
}

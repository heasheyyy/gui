#define IMGUI_DEFINE_MATH_OPERATORS

#include "../menu.hpp"
#include <imgui_ext/animations.hpp>
#include <imgui/imgui_internal.h>

void Menu::login_page() {
	const auto window = ImGui::GetCurrentWindow();
	const auto draw_list = window->DrawList;
	const ImVec2 win_pos = window->Pos;
	const ImVec2 win_size = window->Size;

	const auto style = ImGui::GetStyle();

	static char login_buf[48], password_buf[48];
	ImGui::SetCursorPos(ImVec2(128, 185.5f));
	InputText(local("login"), ICON_FA_USER, login_buf, IM_ARRAYSIZE(login_buf), ImVec2(280, 33));

	ImGui::SetCursorPos(ImVec2(128, 226.5f));
	InputText(local("password"), ICON_FA_LOCK, password_buf, IM_ARRAYSIZE(password_buf), ImVec2(280, 33), ImGuiInputTextFlags_Password);

	const std::string donthaveacc_str = local("donthaveacc").str;
	const std::string signup_str = local("signup").str;
	ImGui::PushFont(font(FONTS::SFPRO_REGULAR));
	const float donthave_width = ImGui::CalcTextSize(donthaveacc_str.c_str()).x;
	ImGui::PopFont();
	const float signup_width = ImGui::CalcTextSize(signup_str.c_str()).x;

	const float text_width = donthave_width + 2.f + signup_width;

	ImGui::SetCursorPos(ImVec2((win_size.x - text_width) * 0.5f, 271.5f));
	ImGui::PushFont(font(FONTS::SFPRO_REGULAR));
	ImGui::Text(donthaveacc_str.c_str());
	ImGui::PopFont();

	ImGui::SameLine(0.f, 2.f);
	ImGui::TextColored(color(COLORS::MAIN), signup_str.c_str());
	if (ImGui::IsItemHovered()) {
		GImGui->MouseCursor = ImGuiMouseCursor_Hand;
		draw_list->AddShadowRect(win_pos + ImVec2((win_size.x - text_width) * 0.5f + donthave_width + 2.f, 271.5f), win_pos + ImVec2((win_size.x - text_width) * 0.5f + donthave_width + 2.f + signup_width, 271.5f + ImGui::CalcTextSize(signup_str.c_str()).y), u32color(COLORS::MAIN, 0.15f), 10.f, ImVec2(0, 0));
	}
	if (ImGui::IsItemClicked())
		ShellExecute(0, 0, L"https://sysreset.xyz", 0, 0, SW_SHOW);

	ImGui::SetCursorPos(ImVec2((win_size.x - 180.f) * 0.5f, 312.5f));
	if (EnterButton(local("enter"), ICON_FA_RIGHT_TO_BRACKET, ImVec2(180, 33))) {
		//do_logo_anim = true;
		set_page(PAGES::MAINS);
	}
}
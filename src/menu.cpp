#define IMGUI_DEFINE_MATH_OPERATORS

#include "menu.hpp"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui_ext/animations.hpp>
#include <windows.h>
#include <shellapi.h>

using namespace ImAnim;
using namespace std::chrono;


void Menu::draw() {
	auto startup_anim_builder = AnimBuilder<float>();
	startup_anim_builder.range(0.f, 1.f).easing(Easing::CubicEaseIn).play_policy(PlayPolicy::ONLY_ONCE).duration(milliseconds(350));
	float startup_grow = startup_anim_builder.build("Startup/Grow", ImGui::GetIO().DeltaTime);

	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, startup_grow);

	ImVec2 target_win_size = ImVec2{ 536, 459 };
	ImVec2 win_size(target_win_size.x, target_win_size.y * startup_grow);
	constexpr auto window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove;

	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
	ImGui::SetNextWindowSize(win_size);
	ImGui::Begin("Spoofer", nullptr, window_flags);

	const auto window = ImGui::GetCurrentWindow();
	const auto draw_list = window->DrawList;
	const ImVec2 win_pos = window->Pos;
	const auto style = ImGui::GetStyle();

	// BACKGROUND SHADOW
	const float shadow_anim = (std::sin(GImGui->Time * 1.45f) + 1.0f) * 0.5f;
	draw_list->AddShadowCircle(win_pos + ImVec2(268, 230), 120, u32color(COLORS::MAIN, 0.15f + 0.15f * shadow_anim), 460.f, ImVec2(0, 0));
	draw_list->AddShadowCircle(win_pos + ImVec2(268, 230), 1, u32color(COLORS::MAIN, 0.18f + 0.15f * shadow_anim), 460.f + 100.f * shadow_anim, ImVec2(0, 0));
	//

	auto main_page_anim = AnimBuilder<float>();
	//main_page_anim.range(0.f, 1.f).easing(Easing::QuadraticEaseInOut).condition(do_logo_anim).play_policy(PlayPolicy::ONLY_ONCE).duration(milliseconds(1000));
	float main_page_anim_v = main_page_anim.build("MainPage", ImGui::GetIO().DeltaTime);

	bool mpcond = main_page_anim_v == 1.f;
	auto main_page_content_anim = AnimBuilder<float>();
	main_page_content_anim.range(0.f, 1.f).easing(Easing::QuadraticEaseInOut).condition(mpcond).play_policy(PlayPolicy::ONLY_ONCE).duration(milliseconds(1000));
	float main_page_content_anim_v = main_page_content_anim.build("MainPageContent", ImGui::GetIO().DeltaTime);

	// HEADER
	{
		if (mpcond) {
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, main_page_content_anim_v);
			if (IconButton(ICON_FA_GEAR))
				set_page(current_page == PAGES::MAINS ? PAGES::SETTINGS : PAGES::MAINS);
			ImGui::PopStyleVar();
		}

		const ImVec2 lang_button_pos = style.WindowPadding + ImVec2((16.f + style.ItemSpacing.x) * main_page_content_anim_v, 0);

		auto language_change_anim_builder = AnimBuilder<float>();
		language_change_anim_builder.range(1.f, 0.f).easing(Easing::CubicEaseIn).duration(milliseconds(200)).condition(language_changed_animation).play_policy(PlayPolicy::ENDLESS);
		float lchange_alpha = language_change_anim_builder.build("LanguageChangeButton", ImGui::GetIO().DeltaTime);
		if (lchange_alpha == 0.f) {
			language_changed_animation = false;
			current_local = next_local;
		}

		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, lchange_alpha);
		ImGui::SetCursorPos(lang_button_pos);
		LanguageButton("languagebutton", image(local("languagebutton", false).ch()), ImVec2(24, 18), true);
		ImGui::PopStyleVar();

		const float buttons_offset = ImGui::CalcTextSize(ICON_FA_MINUS).x + ImGui::CalcTextSize(ICON_FA_XMARK).x + style.ItemSpacing.x;

		ImGui::SetCursorPos(ImVec2(ImGui::GetContentRegionMax().x - buttons_offset, style.WindowPadding.y));
		IconButton(ICON_FA_MINUS);

		ImGui::SameLine();
		if (IconButton(ICON_FA_XMARK))
			std::exit(1);

		const ImVec2 logo_pos = ImVec2(251.f - 41.f * main_page_anim_v, 123.f - 96.f * main_page_anim_v);
		const ImVec2 logo_size = ImVec2(49.f - 18.f * main_page_anim_v, 44.f - 15.f * main_page_anim_v);

		//ImGui::SetCursorPos(logo_pos);
		//ImGui::Image(image("logo"), logo_size, ImVec2(0, 0), ImVec2(1, 1), color(COLORS::WHITE));

			ImGui::SetCursorPos(ImVec2(220.5f + 10.f * main_page_content_anim_v, 24.f));
			ImGui::PushFont(font(FONTS::SFPRO_TITLE));
			ImGui::TextColored(color(COLORS::WHITE, main_page_content_anim_v), "Sysreset");
			ImGui::PopFont();
	}
	// ---

	// Page switch animation
	bool cond = current_page != next_page;
	auto page_alpha_builder = AnimBuilder<float>();
	page_alpha_builder.range(1.f, 0.f).condition(cond).easing(Easing::CubicEaseInOut).duration(milliseconds(500)).play_policy(PlayPolicy::ENDLESS);
	page_alpha = page_alpha_builder.build("PageSwitch", ImGui::GetIO().DeltaTime);
	if (page_alpha == 0.f) {
		current_page = next_page;
	}

	// PAGE
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, page_alpha * startup_grow);
	switch (current_page) {
		case PAGES::LOGIN:
			login_page();
			break;
		case PAGES::MAINS:
			main_page();
			break;
		case PAGES::SETTINGS:
			settings_page();
			break;
	}
	ImGui::PopStyleVar();

	// FOOTER
	{
		const ImRect footer_rect(ImVec2(0, 410), win_size);
		draw_list->AddRectFilled(win_pos + footer_rect.Min, win_pos + footer_rect.Max, u32color(COLORS::FOREGROUND), style.WindowRounding, ImDrawFlags_RoundCornersBottom);

		ImGui::PushFont(font(FONTS::SFPRO_SMALL_MEDIUM));
		draw_list->AddText(win_pos + footer_rect.Min + ImVec2(style.WindowPadding.x, (footer_rect.GetHeight() - ImGui::CalcTextSize("V 3.0").y) * 0.5f), u32color(COLORS::SECONDARY), "V 3.0");

		draw_list->AddText(win_pos + footer_rect.Min  - ImVec2(0, 5.f * (1.f - page_alpha)) + (footer_rect.GetSize() - ImGui::CalcTextSize(get_current_page_name().c_str())) * 0.5f, u32color(COLORS::WHITE, page_alpha), get_current_page_name().c_str());
		ImGui::PopFont();

		ImGui::PushFont(font(FONTS::FA_BRANDS));
		const float buttons_offset = ImGui::CalcTextSize(ICON_FA_DISCORD).x + ImGui::CalcTextSize(ICON_FA_TELEGRAM).x + style.ItemSpacing.x;
		ImGui::SetCursorPos(ImVec2(footer_rect.Max.x - style.WindowPadding.x - buttons_offset, footer_rect.GetCenter().y - ImGui::CalcTextSize(ICON_FA_DISCORD).y * 0.5f));
		if (SocialButton(ICON_FA_GOOGLE))
			ShellExecute(0, 0, L"https://sysreset.xyz", 0, 0, SW_SHOW);

		ImGui::SameLine();
		if (SocialButton(ICON_FA_DISCORD))
			ShellExecute(0, 0, L"https://sysreset.xyz/discord", 0, 0, SW_SHOW);
		ImGui::PopFont();
	}
	// ---

	ImGui::PopStyleVar();
	ImGui::End();
}

Menu::ColorTheme default_colorscheme() {
	Menu::ColorTheme dark;
	dark[Menu::COLORS::BACKGROUND] = ImColor(4, 8, 15, 252);
	dark[Menu::COLORS::FOREGROUND] = ImColor(17, 20, 27, 190);
	dark[Menu::COLORS::DISABLED] = ImColor(24, 28, 35);
	dark[Menu::COLORS::MAIN] = ImColor(50, 145, 243);
	dark[Menu::COLORS::SECONDARY] = ImColor(44, 48, 55);
	dark[Menu::COLORS::WHITE] = ImColor(218, 227, 229);

	auto& colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_WindowBg] = dark[Menu::COLORS::BACKGROUND];
	colors[ImGuiCol_PopupBg] = ImColor(255, 255, 255, 5);
	colors[ImGuiCol_ChildBg] = ImColor(255, 255, 255, 5);
	colors[ImGuiCol_Border] = dark[Menu::COLORS::SECONDARY];
	colors[ImGuiCol_FrameBg] = dark[Menu::COLORS::BACKGROUND];
	colors[ImGuiCol_Text] = dark[Menu::COLORS::WHITE];
	colors[ImGuiCol_TextSelectedBg] = dark[Menu::COLORS::MAIN];

	return dark;
}

void Menu::setup_colors() {
	auto& style = ImGui::GetStyle();
	style.WindowBorderSize = 0;
	style.WindowRounding = 16.f;

	style.ChildRounding = 16.f;
	style.FrameRounding = 8.f;
	style.PopupRounding = 4.f;
	style.WindowMinSize = ImVec2(0.f, 0.f);

	style.ChildBorderSize = 2.f;
	style.FramePadding = ImVec2(10.f, 20.f);
	style.WindowPadding = ImVec2(24.f, 24.f);
	style.ItemSpacing = ImVec2(12, 8);

	current_colors = default_colorscheme();
}

#include <fonts/sfpro_regular.h>
#include <fonts/sfpro_medium.h>
#include <fonts/fa6pro_regular.h>
#include <fonts/fa_brands.h>

void Menu::setup_fonts() {
	ImGuiIO& io = ImGui::GetIO();
	static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	static const ImWchar icon_brands_ranges[] = { ICON_MIN_FAB, ICON_MAX_FAB, 0 };

	ImFontConfig icon_config;
	icon_config.MergeMode = true;
	icon_config.PixelSnapH = true;
	icon_config.OversampleH = 1;

	ImFontConfig font_config;
	font_config.PixelSnapH = true;
	font_config.OversampleH = 1;

	io.Fonts->AddFontFromMemoryCompressedTTF(SFProMedium_compressed_data, SFProMedium_compressed_size, 17.f, &font_config, io.Fonts->GetGlyphRangesCyrillic());
	io.Fonts->AddFontFromMemoryCompressedTTF(FA6ProRegular_compressed_data, FA6ProRegular_compressed_size, 16.f, &icon_config, icon_ranges);

	fonts[FONTS::SFPRO_REGULAR] = io.Fonts->AddFontFromMemoryCompressedTTF(SFProRegular_compressed_data, SFProRegular_compressed_size, 17.f, &font_config, io.Fonts->GetGlyphRangesCyrillic());
	fonts[FONTS::SFPRO_MEDIUM] = io.Fonts->AddFontFromMemoryCompressedTTF(SFProMedium_compressed_data, SFProMedium_compressed_size, 17.f, &font_config, io.Fonts->GetGlyphRangesCyrillic());
	fonts[FONTS::SFPRO_TITLE] = io.Fonts->AddFontFromMemoryCompressedTTF(SFProMedium_compressed_data, SFProMedium_compressed_size, 19.f, &font_config, io.Fonts->GetGlyphRangesCyrillic());
	
	fonts[FONTS::SFPRO_SMALL_REGULAR] = io.Fonts->AddFontFromMemoryCompressedTTF(SFProRegular_compressed_data, SFProRegular_compressed_size, 15.f, &font_config, io.Fonts->GetGlyphRangesCyrillic());
	fonts[FONTS::SFPRO_SMALL_REGULAR] = io.Fonts->AddFontFromMemoryCompressedTTF(FA6ProRegular_compressed_data, FA6ProRegular_compressed_size, 15.f, &icon_config, icon_ranges);
	fonts[FONTS::SFPRO_SMALL_MEDIUM] = io.Fonts->AddFontFromMemoryCompressedTTF(SFProMedium_compressed_data, SFProMedium_compressed_size, 15.f, &font_config, io.Fonts->GetGlyphRangesCyrillic());

	fonts[FONTS::FA_REGULAR] = io.Fonts->AddFontFromMemoryCompressedTTF(FA6ProRegular_compressed_data, FA6ProRegular_compressed_size, 16.f, &font_config, icon_ranges);
	fonts[FONTS::FA_REGULAR_BIG] = io.Fonts->AddFontFromMemoryCompressedTTF(FA6ProRegular_compressed_data, FA6ProRegular_compressed_size, 72.0f, &font_config, icon_ranges);
	fonts[FONTS::FA_BRANDS] = io.Fonts->AddFontFromMemoryCompressedTTF(FABrands_compressed_data, FABrands_compressed_size, 16.f, &font_config, icon_brands_ranges);

	io.Fonts->Flags |= ImFontAtlasFlags_NoPowerOfTwoHeight;
	io.Fonts->Flags |= ImFontAtlasFlags_NoBakedLines;

	io.Fonts->Build();
}

#include <images/us.h>
#include <images/ru.h>
#include <images/logo.h>
#include <images/cs2.h>
#include <images/dota2.h>
#include <images/fivem.h>
#include <images/minecraft.h>
void Menu::setup_images(ID3D11Device* device) {
	D3DX11_IMAGE_LOAD_INFO iInfo;
	ID3DX11ThreadPump* threadPump{ nullptr };

	D3DX11CreateShaderResourceViewFromMemory(device, us, sizeof(us), &iInfo, threadPump, &images["us"], 0);
	D3DX11CreateShaderResourceViewFromMemory(device, ru, sizeof(ru), &iInfo, threadPump, &images["ru"], 0);
	D3DX11CreateShaderResourceViewFromMemory(device, logo, sizeof(logo), &iInfo, threadPump, &images["logo"], 0);

	D3DX11CreateShaderResourceViewFromMemory(device, cs2, sizeof(cs2), &iInfo, threadPump, &images["cs2"], 0);
	D3DX11CreateShaderResourceViewFromMemory(device, dota2, sizeof(dota2), &iInfo, threadPump, &images["dota2"], 0);
	D3DX11CreateShaderResourceViewFromMemory(device, fivem, sizeof(fivem), &iInfo, threadPump, &images["fivem"], 0);
	D3DX11CreateShaderResourceViewFromMemory(device, minecraft, sizeof(minecraft), &iInfo, threadPump, &images["minecraft"], 0);
}

ImVec4 Menu::color(COLORS color, float alpha) const {
	auto& col = current_colors.at(color);
	return ImVec4(col.x, col.y, col.z, alpha == 1.f ? col.w : alpha);
}

ImU32 Menu::u32color(COLORS color, float alpha) const {
	ImVec4 from_color = current_colors.at(color);
	return ImGui::GetColorU32(ImVec4(from_color.x, from_color.y, from_color.z, alpha == 1.f ? from_color.w : alpha));
}

ImFont* Menu::font(FONTS font) {
	return fonts[font];
}

ImTextureID Menu::image(std::string_view name) const {
	return (ImTextureID)(intptr_t)(images.at(name));
}

void Menu::set_page(PAGES new_page) {
	if (current_page == new_page)
		return;

	next_page = new_page;
}

std::string Menu::get_current_page_name() {
	switch (current_page) {
	case PAGES::LOGIN:
		return local("loginpage").str;
		break;
	case PAGES::MAINS:
		return local("homepage").str;
		break;
	case PAGES::SETTINGS:
		return local("settingspage").str;
		break;
	}
}

#include <random>

std::string getRandomChar() {
	const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	std::default_random_engine generator(std::random_device{}());
	std::uniform_int_distribution<int> distribution(0, sizeof(charset) - 2);
	return std::string(1, charset[distribution(generator)]);
}

size_t utf8_length(const std::string& str) {
	unsigned int strLen = str.length();
	unsigned int u = 0;
	const char* c_str = str.c_str();
	unsigned int charCount = 0;
	while (u < strLen)
	{
		u += mblen(&c_str[u], strLen - u);
		charCount += 1;
	}
	return charCount;
}
std::map<size_t, size_t>& Menu::get_locals_data() {
	static std::map<size_t, size_t> locals_data;
	return locals_data;
}

std::map<size_t, float>& Menu::get_locals_time() {
	static std::map<size_t, float> locals_time;
	return locals_time;
}

std::string Menu::animate_string(const size_t str_id, const std::string& local_str, float delta_time) {
	const auto length = utf8_length(local_str);
	std::string current_string(length, ' ');

	auto& locals_data = get_locals_data();
	auto& locals_time = get_locals_time();
	if (reset_string_animations) {
		locals_time.clear();
		locals_data.clear();
		reset_string_animations = false;
	}

	size_t& current_idx = locals_data[str_id];
	float& elapsed_time = locals_time[str_id];

	elapsed_time += delta_time;

	const float animation_speed = ImGui::GetIO().DeltaTime * 80.f / length;

	if (elapsed_time >= animation_speed) {
		if (current_idx < length) {
			current_idx++;
		}
		elapsed_time = 0.0f;
	}

	for (size_t i = 0; i < length; ++i) {
		if (i < current_idx) {
			current_string[i] = local_str[i];
		}
		else {
			current_string[i] = getRandomChar()[0];
		}
	}

	return current_string;
}

Local Menu::local(const char* id, bool animate) {
	const size_t id_hash = hash(id);
	std::string return_str;
	for (auto& str : localized_strings) {
		if (str.id != id_hash)
			continue;

		switch (current_local) {
		case LOCALS::ENGLISH:
			return_str = str.en;
			break;
		case LOCALS::RUSSIAN:
			return_str = str.ru;
			break;
		}
	}

	if (animate) {
		return_str = animate_string(id_hash, return_str, ImGui::GetIO().DeltaTime);
	}

	return Local{ id_hash, std::move(return_str) };
}

void Menu::change_local(LOCALS new_local) {
	if (current_local == new_local)
		return;

	next_local = new_local;
	reset_string_animations = true;
	language_changed_animation = true;
}

#include <imgui_ext/helpers.hpp>

void Menu::CloseCurrentPopupAnimated() {
	ImHelpers::ClosePopupAnimated(current_popup_animated_id);
}
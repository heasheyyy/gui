#pragma once
#include <imgui/imgui.h>
#include <map>
#include <string>

#include <d3d11.h>
#include <d3dx11.h>
#pragma comment (lib, "d3dx11.lib")

#include <fonts/IconsFontAwesome6Pro.h>
#include <fonts/IconsFontAwesome6Brands.h>
#include <locals.hpp>

using namespace std::string_literals;

using TextureType = ID3D11ShaderResourceView*;

class Menu {
public:
	enum class LOCALS {
		ENGLISH,
		RUSSIAN
	};
	enum class PAGES {
		LOGIN,
		MAINS,
		SETTINGS
	};
	enum class FONTS {
		SFPRO_REGULAR,
		SFPRO_MEDIUM,
		SFPRO_TITLE,
		SFPRO_SMALL_REGULAR,
		SFPRO_SMALL_MEDIUM,
		FA_REGULAR,
		FA_REGULAR_BIG,
		FA_BRANDS
	};
	enum class COLORS {
		MAIN,
		BACKGROUND,
		WHITE,
		DISABLED,
		FOREGROUND,
		SECONDARY
	};

	using ColorTheme = std::map<COLORS, ImVec4>;

private:
	std::map<FONTS, ImFont*> fonts;
	std::map<std::string_view, TextureType> images;

	ColorTheme current_colors;

	LOCALS current_local = LOCALS::ENGLISH;
	LOCALS next_local = LOCALS::ENGLISH;

	PAGES current_page = PAGES::LOGIN;
	PAGES next_page = PAGES::LOGIN;
	float page_alpha = 1.f;

	bool reset_string_animations = true;
	bool language_changed_animation = false;

	//bool do_logo_anim = false;

	const char* current_popup_animated_id = nullptr;

public:
	void draw();

	void setup_colors();
	void setup_fonts();
	void setup_images(ID3D11Device* device);

	[[nodiscard]] ImVec4 color(COLORS color, float alpha = 1.f) const;
	[[nodiscard]] ImU32 u32color(COLORS color, float alpha = 1.f) const;
	[[nodiscard]] ImFont* font(FONTS font);
	[[nodiscard]] ImTextureID image(std::string_view name) const;
	[[nodiscard]] Local local(const char* id, bool animate = true);
private:
	void set_page(PAGES new_page);
	std::string get_current_page_name();

	void change_local(LOCALS new_local);

	std::map<size_t, size_t>& get_locals_data();
	std::map<size_t, float>& get_locals_time();

	std::string animate_string(const size_t str_id, const std::string& local_str, float delta_time);


	void login_page();
	void main_page();
	void settings_page();

	void CloseCurrentPopupAnimated();
public:
	void BeginChild(const char* str_id, const ImVec2& size = ImVec2(0, 0), ImGuiChildFlags child_flags = 0, ImGuiWindowFlags window_flags = 0);
	void EndChild();

	bool Checkbox(const Local& label, const char* icon, bool& value);

	bool BeginCombo(const char* str_id, const char* preview_value, size_t items_count, ImTextureID preview_image, ImGuiComboFlags flags = 0);
	void EndCombo(const char* str_id);
	bool Selectable(const char* label, ImTextureID user_texture_id, bool selected, ImGuiSelectableFlags flags = 0, const ImVec2& size_arg = ImVec2(0, 0));
	bool Combo(const char* str_id, int& current_item, std::initializer_list<const char*> items, std::initializer_list<ImTextureID> item_images);

	bool LanguageButton(const char* str_id, ImTextureID image, const ImVec2& size, bool is_main_button = false);
	bool IconButton(const char* icon);
	bool SocialButton(const char* icon);
	bool EnterButton(const Local& label, const char* icon, const ImVec2& size_arg = ImVec2(0, 0));
	bool SpoofButton(const char* icon, const ImVec2& size, bool& active, bool target);

	bool InputText(const Local& label, const char* icon, char* buf, int buf_size, const ImVec2& size_arg, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* callback_user_data = nullptr);
};
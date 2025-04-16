#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace ImHelpers {
    // Rounded multicolorrect
    void AddRectFilledMultiColor(ImDrawList* drawlist, const ImVec2& p_min, const ImVec2& p_max, ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left, float rounding, ImDrawFlags flags = 0);

    // Patched popup workaround for popups, that add close-animations functionality
    void BeginPopup(const char* str_id, ImGuiWindowFlags flags = 0);
    void EndPopup();

    enum class PopupAnimStyles {
        TOPLEFT,
        TOP,
        BOTTOM
    };

    bool BeginPopupAnimated(const char* str_id, bool pressed_state, const ImVec2& pos, const ImVec2& size = ImVec2(0, 0), PopupAnimStyles style = PopupAnimStyles::TOPLEFT);
    void EndPopupAnimated(const char* str_id);
    bool IsPopupAnimatedOpen(const char* str_id);
    void ClosePopupAnimated(const char* str_id);
}
#include "helpers.hpp"
#include <imgui_ext/animations.hpp>
#include <map>

using namespace ImAnim;
using namespace std::chrono;
using namespace std::string_literals;

void ImHelpers::AddRectFilledMultiColor(ImDrawList* drawlist, const ImVec2& p_min, const ImVec2& p_max, ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left, float rounding, ImDrawFlags flags)
{
    if (((col_upr_left | col_upr_right | col_bot_right | col_bot_left) & IM_COL32_A_MASK) == 0)
        return;

    rounding = ImMin(rounding, ImFabs(p_max.x - p_min.x) * (((flags & ImDrawFlags_RoundCornersTop) == ImDrawFlags_RoundCornersTop) || ((flags & ImDrawFlags_RoundCornersBottom) == ImDrawFlags_RoundCornersBottom) ? 0.5f : 1.0f) - 1.0f);
    rounding = ImMin(rounding, ImFabs(p_max.y - p_min.y) * (((flags & ImDrawFlags_RoundCornersLeft) == ImDrawFlags_RoundCornersLeft) || ((flags & ImDrawFlags_RoundCornersRight) == ImDrawFlags_RoundCornersRight) ? 0.5f : 1.0f) - 1.0f);

    // https://github.com/ocornut/imgui/issues/3710#issuecomment-758555966
    if (rounding > 0.0f && flags != ImDrawFlags_None)
    {
        const int size_before = drawlist->VtxBuffer.Size;
        drawlist->AddRectFilled(p_min, p_max, IM_COL32_WHITE, rounding, flags);
        const int size_after = drawlist->VtxBuffer.Size;

        for (int i = size_before; i < size_after; i++)
        {
            ImDrawVert* vert = drawlist->VtxBuffer.Data + i;

            ImVec4 upr_left = ImGui::ColorConvertU32ToFloat4(col_upr_left);
            ImVec4 bot_left = ImGui::ColorConvertU32ToFloat4(col_bot_left);
            ImVec4 up_right = ImGui::ColorConvertU32ToFloat4(col_upr_right);
            ImVec4 bot_right = ImGui::ColorConvertU32ToFloat4(col_bot_right);

            float X = ImClamp((vert->pos.x - p_min.x) / (p_max.x - p_min.x), 0.0f, 1.0f);

            // 4 colors - 8 deltas

            float r1 = upr_left.x + (up_right.x - upr_left.x) * X;
            float r2 = bot_left.x + (bot_right.x - bot_left.x) * X;

            float g1 = upr_left.y + (up_right.y - upr_left.y) * X;
            float g2 = bot_left.y + (bot_right.y - bot_left.y) * X;

            float b1 = upr_left.z + (up_right.z - upr_left.z) * X;
            float b2 = bot_left.z + (bot_right.z - bot_left.z) * X;

            float a1 = upr_left.w + (up_right.w - upr_left.w) * X;
            float a2 = bot_left.w + (bot_right.w - bot_left.w) * X;


            float Y = ImClamp((vert->pos.y - p_min.y) / (p_max.y - p_min.y), 0.0f, 1.0f);
            float r = r1 + (r2 - r1) * Y;
            float g = g1 + (g2 - g1) * Y;
            float b = b1 + (b2 - b1) * Y;
            float a = a1 + (a2 - a1) * Y;
            ImVec4 RGBA(r, g, b, a);

            RGBA = RGBA * ImGui::ColorConvertU32ToFloat4(vert->col);

            vert->col = ImColor(RGBA);
        }
        return;
    }

    const ImVec2 uv = drawlist->_Data->TexUvWhitePixel;
    drawlist->PrimReserve(6, 4);
    drawlist->PrimWriteIdx((ImDrawIdx)(drawlist->_VtxCurrentIdx)); drawlist->PrimWriteIdx((ImDrawIdx)(drawlist->_VtxCurrentIdx + 1)); drawlist->PrimWriteIdx((ImDrawIdx)(drawlist->_VtxCurrentIdx + 2));
    drawlist->PrimWriteIdx((ImDrawIdx)(drawlist->_VtxCurrentIdx)); drawlist->PrimWriteIdx((ImDrawIdx)(drawlist->_VtxCurrentIdx + 2)); drawlist->PrimWriteIdx((ImDrawIdx)(drawlist->_VtxCurrentIdx + 3));
    drawlist->PrimWriteVtx(p_min, uv, col_upr_left);
    drawlist->PrimWriteVtx(ImVec2(p_max.x, p_min.y), uv, col_upr_right);
    drawlist->PrimWriteVtx(p_max, uv, col_bot_right);
    drawlist->PrimWriteVtx(ImVec2(p_min.x, p_max.y), uv, col_bot_left);
}

void ImHelpers::BeginPopup(const char* str_id, ImGuiWindowFlags flags) {
    const auto popup_flags = flags | ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    ImGui::SetNextWindowFocus();
    ImGui::Begin("LanguagePopup", nullptr, popup_flags);
}
void ImHelpers::EndPopup() {
    ImGui::End();
}

struct PopupAnimStates {
    bool target_visible_state = false;
    bool pressed_state = false;
    bool is_opened = false;
};

auto& get_popup_animated_states() {
    static std::map<const char*, PopupAnimStates> popup_open_states;
    return popup_open_states;
}

bool ImHelpers::IsPopupAnimatedOpen(const char* str_id) {
    auto& popup_anim_states = get_popup_animated_states();
    if (auto it = popup_anim_states.find(str_id); it != popup_anim_states.end()) {
        return it->second.is_opened;
    }
    return false;
}

void ImHelpers::ClosePopupAnimated(const char* str_id) {
    auto& popup_open_states = get_popup_animated_states();
    popup_open_states[str_id].target_visible_state = false;
}

bool ImHelpers::BeginPopupAnimated(const char* str_id, bool pressed_state, const ImVec2& pos, const ImVec2& size, PopupAnimStyles style) {
    auto& popup_open_states = get_popup_animated_states();

    popup_open_states[str_id].pressed_state = pressed_state;
    if (pressed_state) {
        popup_open_states[str_id].target_visible_state = true;
    }

    std::string anim_name = "BeginPopupAnimated"s + str_id;
    auto anim_builder = AnimBuilder<float>().range(0.f, 1.f).condition(popup_open_states[str_id].target_visible_state).duration(milliseconds(350)).easing(Easing::QuadraticEaseInOut).play_policy(PlayPolicy::ENDLESS);
    float popup_size_modif = anim_builder.build(anim_name, ImGui::GetIO().DeltaTime);

    bool is_popup_opened = (popup_size_modif != 0.f);
    popup_open_states[str_id].is_opened = is_popup_opened;
    if (!is_popup_opened)
        return false;

    const ImVec2 next_win_size = style == PopupAnimStyles::TOPLEFT ? (size * popup_size_modif) : ImVec2(size.x, size.y * popup_size_modif);
    ImGui::SetNextWindowPos(style == PopupAnimStyles::BOTTOM ? ImVec2(pos.x, pos.y + size.y * (1.f - popup_size_modif)) : pos);
    ImGui::SetNextWindowSize(next_win_size);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, popup_size_modif);
    ImHelpers::BeginPopup(str_id);

    return true;
}

void ImHelpers::EndPopupAnimated(const char* str_id) {
    const bool is_popup_hovered = ImGui::IsWindowHovered();
    ImHelpers::EndPopup();
    ImGui::PopStyleVar();

    auto& current_popup_anim_state = get_popup_animated_states()[str_id];
    if (!current_popup_anim_state.pressed_state && (ImGui::IsKeyPressed(ImGuiKey_Escape) || ImGui::IsMouseReleased(ImGuiMouseButton_Left)) && !is_popup_hovered) {
        current_popup_anim_state.target_visible_state = false;
    }
}
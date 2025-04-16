#define IMGUI_DEFINE_MATH_OPERATORS

#include "menu.hpp"
#include <imgui/imgui_internal.h>
#include <imgui_ext/animations.hpp>
#include <imgui_ext/helpers.hpp>
#include <imgui_ext/rotate.hpp>

using namespace ImAnim;
using namespace std::chrono;

void Menu::BeginChild(const char* str_id, const ImVec2& size, ImGuiChildFlags child_flags, ImGuiWindowFlags window_flags) {
    const auto window = ImGui::GetCurrentWindow();
    const ImVec2 win_pos = window->Pos;
    const auto draw_list = ImGui::GetCurrentWindow()->DrawList;
    const ImVec2 child_pos = window->DC.CursorPos;
    const auto style = ImGui::GetStyle();
    const float child_rounding = style.ChildRounding;
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(17.34f, 18.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.f, 0.f, 0.f, 0.f));
    ImGui::BeginChild(str_id, size, child_flags | ImGuiChildFlags_Border, window_flags | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    int vert_start_idx = window->DrawList->VtxBuffer.Size;
    draw_list->AddRect(child_pos + ImVec2(0.75f, 0.75f), child_pos + size - ImVec2(0.75f, 0.75f), ImColor(255, 255, 255, int(255.f * style.Alpha)), child_rounding * 0.95f, 0, 1.5f);
    int vert_end_idx = window->DrawList->VtxBuffer.Size;
    ImGui::ShadeVertsLinearColorGradientKeepAlpha(window->DrawList, vert_start_idx, vert_end_idx, child_pos, child_pos + size * 0.1f, ImColor(17, 22, 30), ImColor(24, 28, 35)); // ImColor(17, 22, 30), ImColor(24, 28, 35)
}
void Menu::EndChild() {
    ImGui::EndChild();
}

bool Menu::Checkbox(const Local& label, const char* icon, bool& value) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label.id);
    const ImVec2 label_size = ImGui::CalcTextSize(label.ch(), NULL, true);
    const ImVec2 icon_size = ImGui::CalcTextSize(icon, NULL, true);

    const ImVec2 pos = window->DC.CursorPos;
    const ImRect total_bb(pos, pos + ImVec2(ImGui::GetContentRegionAvail().x, ImMax(label_size.y, icon_size.y)));
    ImGui::ItemSize(total_bb, style.FramePadding.y);
    const bool is_visible = ImGui::ItemAdd(total_bb, id);
    if (!is_visible) {
        if (!g.BoxSelectState.UnclipMode || !g.BoxSelectState.UnclipRect.Overlaps(total_bb)) // Extra layer of "no logic clip" for box-select support
        {
            IMGUI_TEST_ENGINE_ITEM_INFO(id, label.ch(), g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
            return false;
        }
    }

    bool checked = value;
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);

    if (pressed)
        checked = !checked;

    if (value != checked)
    {
        value = checked;
        pressed = true; // return value
        ImGui::MarkItemEdited(id);
    }

    const float check_size = 15.f;
    const ImRect check_bb(ImVec2(total_bb.Max.x - check_size, total_bb.GetCenter().y - check_size * 0.5f),
        ImVec2(total_bb.Max.x, total_bb.GetCenter().y + check_size * 0.5f));
    if (is_visible)
    {
        ImGui::RenderNavCursor(total_bb, id);


        // Animate border
        std::string border_anim_name = "Checkbox/Border/Alpha#"s + std::to_string(label.id);
        std::string border_grow_anim_name = "Checkbox/Border/Grow#"s + std::to_string(label.id);

        bool anim_condition = value;
        auto border_builder = AnimBuilder<float>().range(0.f, 1.f).condition(anim_condition).duration(milliseconds(200)).easing(Easing::QuadraticEaseIn).play_policy(PlayPolicy::ENDLESS);
        float border = border_builder.build(border_anim_name, ImGui::GetIO().DeltaTime);

        auto border_grow_builder = AnimBuilder<float>().range(0.f, 1.f).condition(anim_condition).duration(milliseconds(250)).easing(Easing::BackEaseOut).play_policy(PlayPolicy::ENDLESS);
        float border_grow = border_grow_builder.build(border_grow_anim_name, ImGui::GetIO().DeltaTime);

        // Animate grow
        bool anim_condition2 = border == 1.f;
        std::string frame_grow_anim_name = "Checkbox/FrameGrow#"s + std::to_string(label.id);
        auto frame_grow_builder = AnimBuilder<float>().range(0.f, 1.f).play_policy(PlayPolicy::ENDLESS).easing(Easing::QuadraticEaseOut).duration(milliseconds(150));
        frame_grow_builder.condition(anim_condition2);
        float frame_grow = frame_grow_builder.build(frame_grow_anim_name, ImGui::GetIO().DeltaTime);

        // Render frame
        ImRect frame_rect(check_bb.Min + ImVec2(check_size * 0.5f * (1.f - frame_grow), check_size * 0.5f * (1.f - frame_grow)),
            check_bb.Max - ImVec2(check_size * 0.5f * (1.f - frame_grow), check_size * 0.5f * (1.f - frame_grow)));

        ImGui::RenderFrame(check_bb.Min, check_bb.Max, u32color(COLORS::FOREGROUND), true, 4.f);
        window->DrawList->AddRect(check_bb.Min, check_bb.Max, u32color(COLORS::SECONDARY), 4.f);

        // Render frame
        //ImGui::RenderFrame(total_bb.Min, total_bb.Max, u32color(COLORS::FOREGROUND), false, style.FrameRounding);

        ImRect border_rect(check_bb.Min - ImVec2(5, 5) * (1.f - border_grow), check_bb.Max + ImVec2(5, 5) * (1.f - border_grow));
        window->DrawList->AddRect(border_rect.Min, border_rect.Max, u32color(COLORS::MAIN, border), 4.f, 0, 1.1f);
        ImGui::RenderFrame(check_bb.Min, check_bb.Max, ImGui::GetColorU32(color(COLORS::MAIN, frame_grow)), false, 4.f);

        const float pad = ImMax(1.0f, IM_TRUNC(check_size / 3.75f));
        const float mark_size = check_size - pad * 2.0f;
        const ImVec2 mark_pos = check_bb.GetCenter() - ImVec2(mark_size, mark_size) * 0.5f;

        ImGui::RenderCheckMark(window->DrawList, mark_pos, u32color(COLORS::BACKGROUND, frame_grow), mark_size);
    }

    const ImVec2 icon_pos = total_bb.Min + ImVec2(0, (total_bb.GetHeight() - icon_size.y) * 0.5f);
    const ImVec2 text_pos = total_bb.Min + ImVec2(26.f, (total_bb.GetHeight() - label_size.y) * 0.5f);
    if (g.LogEnabled)
        ImGui::LogRenderedText(&text_pos, value ? "[x]" : "[ ]");
    if (is_visible) {
        ImGui::PushStyleColor(ImGuiCol_Text, color(COLORS::MAIN));
        ImGui::RenderText(icon_pos, icon);
        ImGui::PopStyleColor();

        if (label_size.x > 0.0f)
            ImGui::RenderText(text_pos, label.ch());
    }

    if (hovered)
        g.MouseCursor = ImGuiMouseCursor_Hand;

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label.ch(), g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
    return pressed;
}

bool Menu::LanguageButton(const char* str_id, ImTextureID img, const ImVec2& size, bool is_main_button) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(str_id);
    const ImVec2 pos = window->DC.CursorPos;

    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    std::string glow_anim_name = "LanguageButton#"s + str_id;
    auto glow_builder = AnimBuilder<float>().range(0.f, 1.f).condition(hovered).duration(milliseconds(200)).easing(Easing::QuadraticEaseIn).play_policy(PlayPolicy::ENDLESS);
    float glow = glow_builder.build(glow_anim_name, ImGui::GetIO().DeltaTime);

    window->DrawList->AddImage(img, bb.Min, bb.Max, ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, int(255.f * (is_main_button ? 1.f : (1.f - 0.5f + (glow * 0.5f))) * style.Alpha)));

    if (is_main_button) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.f, 16.f));
        if (ImHelpers::BeginPopupAnimated(str_id, pressed, pos + ImVec2(0, size.y + 4.f), ImVec2(24.f * 2.f + 16.f * 2.f + 8.f, 18.f + 16.f * 2.f))) {
            if (LanguageButton("us_lbuttton", image("us"), ImVec2(24, 18))) {
                change_local(LOCALS::ENGLISH);
                ImHelpers::ClosePopupAnimated(str_id);
            }

            ImGui::SameLine();
            if (LanguageButton("ru_lbuttton", image("ru"), ImVec2(24, 18))) {
                change_local(LOCALS::RUSSIAN);
                ImHelpers::ClosePopupAnimated(str_id);
            }
            ImHelpers::EndPopupAnimated(str_id);
        }
        ImGui::PopStyleVar();
    }

    if (hovered)
        g.MouseCursor = ImGuiMouseCursor_Hand;

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
    return pressed;
}

bool Menu::IconButton(const char* icon) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(icon);
    const ImVec2 icon_size = ImGui::CalcTextSize(icon, NULL, true, 17.f);

    const ImVec2 pos = window->DC.CursorPos;
    const ImRect bb(pos, pos + icon_size);
    ImGui::ItemSize(bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    // Animate Color
    const std::string col_anim_name = "IconButton/Color#"s + icon;
    const ImVec4 default_col = color(COLORS::WHITE);
    const ImVec4 actived_col = color(COLORS::MAIN);
    auto col_builder = AnimBuilder<ImVec4>().range(default_col, actived_col).duration(milliseconds(250)).condition(hovered).play_policy(PlayPolicy::ENDLESS);
    const ImVec4 col = col_builder.build(col_anim_name, ImGui::GetIO().DeltaTime);

    // Animate Rotate
    std::string rotate_anim_name = "IconButton/Rotate#"s + icon;
    auto rotate_builder = AnimBuilder<float>().range(90.f, -270.f).easing(Easing::QuadraticEaseInOut).duration(milliseconds(500)).condition(hovered).play_policy(PlayPolicy::ENDLESS);
    float rotation = rotate_builder.build(rotate_anim_name, ImGui::GetIO().DeltaTime);


    ImGui::PushStyleColor(ImGuiCol_Text, col);

    ImRotations::Start();
    ImGui::RenderTextClipped(bb.Min, bb.Max, icon, NULL, &icon_size, style.ButtonTextAlign, &bb);
    ImRotations::End(ImRotations::deg2rad(rotation));

    ImGui::PopStyleColor();

    if (hovered)
        g.MouseCursor = ImGuiMouseCursor_Hand;

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
    return pressed;
}

bool Menu::EnterButton(const Local& label, const char* icon, const ImVec2& size_arg) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label.id);
    const ImVec2 label_size = ImGui::CalcTextSize(label.ch(), NULL, true);
    const ImVec2 icon_size = ImGui::CalcTextSize(icon, NULL, true);
    const float content_width = label_size.x + 4.f + icon_size.x;

    const ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = ImGui::CalcItemSize(size_arg, content_width + style.FramePadding.x * 2.0f, content_width + style.FramePadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    // Animate Color
    static bool is_entered = false;
    const std::string col_anim_name = "EnterButton/Color#"s + std::to_string(label.id);
    const ImVec4 default_col = color(COLORS::BACKGROUND);
    const ImVec4 actived_col = color(COLORS::BACKGROUND);

    bool anim_cond = is_entered || hovered;
    auto col_builder = AnimBuilder<ImVec4>().range(default_col, actived_col).duration(milliseconds(250)).condition(anim_cond).play_policy(PlayPolicy::ENDLESS).dynamic_range(false);
    const ImVec4 col = col_builder.build(col_anim_name, ImGui::GetIO().DeltaTime);

    // Animate Move
    std::string move_anim_name = "EnterButton/Move#"s + std::to_string(label.id);
    auto move_builder = AnimBuilder<float>().range(0.f, 6.f).easing(Easing::QuadraticEaseInOut).duration(milliseconds(250)).condition(anim_cond).play_policy(PlayPolicy::ENDLESS).dynamic_range(false);
    float move = move_builder.build(move_anim_name, ImGui::GetIO().DeltaTime);

    if (pressed) {
        is_entered = true;
        get_active_anims<ImVec4>()[col_anim_name].reset_to(color(COLORS::MAIN));
        get_active_anims<float>()[move_anim_name].reset_to(48.f);
    }

    ImGui::PushStyleColor(ImGuiCol_Text, col);

    const ImVec2 label_pos = ImVec2(bb.GetCenter().x - content_width * 0.5f - move, bb.GetCenter().y - label_size.y * 0.5f);
    const ImVec2 icon_pos = ImVec2(label_pos.x + label_size.x + 4.f + move * 2.f, bb.GetCenter().y - icon_size.y * 0.5f + 1.25f);

    ImGui::RenderFrame(bb.Min, bb.Max, u32color(COLORS::MAIN), false, 12.f);
    if (col != color(COLORS::MAIN)) {
        ImGui::RenderText(label_pos, label.ch());
        ImGui::RenderText(icon_pos, icon);
    }
    //ImGui::RenderTextClipped(bb.Min, bb.Max, icon, NULL, &icon_size, ImVec2(0, 0), &bb);

    ImGui::PopStyleColor();

    if (hovered)
        g.MouseCursor = ImGuiMouseCursor_Hand;

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label.ch(), g.LastItemData.StatusFlags);
    return pressed;
}

bool Menu::SpoofButton(const char* icon, const ImVec2& size, bool& active, bool target) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(icon);

    ImGui::PushFont(font(FONTS::FA_REGULAR_BIG));
    const ImVec2 icon_size = ImGui::CalcTextSize(icon, NULL, true, 17.f);

    const ImVec2 pos = window->DC.CursorPos;
    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return false;

    static bool is_pressed = false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    if (pressed) {
        is_pressed = true;
    }

    bool cond = is_pressed && (active != target);
    std::string active_anim_name = "SpoofButton/Active#"s + icon;
    auto active_builder = AnimBuilder<float>().range(0.f, 1.f).easing(Easing::QuadraticEaseInOut).duration(milliseconds(400)).condition(cond).play_policy(PlayPolicy::ENDLESS);
    float wait_active = active_builder.build(active_anim_name, ImGui::GetIO().DeltaTime);


    float product = static_cast<float>(g.Time) * 90.0f;
    float rotate = fmod(round(product / 90.0f) * 90.0f, 360.0f);

    std::string rotate_anim_name = "SpoofButton/Rotate"s + icon;
    auto rotate_builder = AnimBuilder<float>().range(180.f, cond ? rotate : 0).easing(Easing::QuadraticEaseInOut).duration(milliseconds(400)).play_policy(PlayPolicy::ENDLESS);
    float rotate_anim = rotate_builder.build(rotate_anim_name, ImGui::GetIO().DeltaTime);

    if (pressed && wait_active != 0.f)
        pressed = false;

    static bool first_use = true;
    std::string passive_anim_name = "SpoofButton/Passive#"s + icon;
    std::string passive2_anim_name = "SpoofButton/Passive2#"s + icon;
    if (pressed && !target) {
        auto& active_anims = get_active_anims<float>();
        if (auto it = active_anims.find(passive_anim_name); it != active_anims.end()) {
            active_anims.erase(passive_anim_name);
            active_anims.erase(passive2_anim_name);
            first_use = true;
        }
    }

    if (active && wait_active == 0.f) {
        constexpr int anim_duration = 8000;
        auto passive_builder = AnimBuilder<float>().range(0.f, 2.25f).easing(Easing::LinearInterpolation).duration(milliseconds(anim_duration));
        float passive = passive_builder.build(passive_anim_name, ImGui::GetIO().DeltaTime);

        auto passive2_builder = AnimBuilder<float>().range(0.f, 2.25f).easing(Easing::LinearInterpolation).duration(milliseconds(anim_duration));
        if (first_use) {
            passive2_builder.delay(milliseconds(static_cast<int>(anim_duration * 0.4375)));
            first_use = false;
        }
        float passive2 = passive2_builder.build(passive2_anim_name, ImGui::GetIO().DeltaTime);

        const float max_radius = size.x * 0.35f;
        const float start_radius = size.x * 0.4f;

        for (size_t i = 0; i < 5; i++) {
            const float currect_passive = ImMin(1.f, ImMax(0.f, passive - static_cast<float>(i) * 0.25f));
            window->DrawList->AddCircle(bb.GetCenter(), style.Alpha * start_radius + max_radius * currect_passive, u32color(COLORS::MAIN, style.Alpha * 1.f - currect_passive), 128, 4.f - 3.f * currect_passive);
        }

        for (size_t i = 1; i < 5; i++) {
            const float currect_passive = ImMin(1.f, ImMax(0.f, passive2 - static_cast<float>(i) * 0.25f));
            window->DrawList->AddCircle(bb.GetCenter(), style.Alpha * start_radius + max_radius * currect_passive, u32color(COLORS::MAIN, style.Alpha * 1.f - currect_passive), 128, 4.f - 3.f * currect_passive);
        }
    }

    const float some_size = size.x * 0.12f;
    ImGui::RenderFrame(bb.Min + ImVec2(some_size, some_size) * wait_active, bb.Max - ImVec2(some_size, some_size) * wait_active, u32color(COLORS::MAIN), false, 128);

    if (wait_active > 0.0f)
        window->DrawList->AddShadowCircle(bb.GetCenter(), size.x * 0.2f, u32color(COLORS::MAIN, wait_active), 235.f, ImVec2(0, 0), 0, 96);
    
        ImRotations::Start();
    
    ImGui::PushStyleColor(ImGuiCol_Text, color(COLORS::BACKGROUND));
    ImGui::RenderTextClipped(bb.Min, bb.Max, icon, NULL, &icon_size, style.ButtonTextAlign, &bb);
    ImGui::PopStyleColor();

        ImRotations::End(ImRotations::deg2rad(-rotate_anim));
    ImGui::PopFont();

    if (hovered)
        g.MouseCursor = ImGuiMouseCursor_Hand;

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
    return pressed;
}

bool Menu::SocialButton(const char* icon) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(icon);
    const ImVec2 icon_size = ImGui::CalcTextSize(icon, NULL, true);

    const ImVec2 pos = window->DC.CursorPos;
    const ImRect bb(pos, pos + icon_size);
    ImGui::ItemSize(bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    // Animate Color
    const std::string col_anim_name = "SocialButton/Color#"s + icon;
    const ImVec4 default_col = color(COLORS::SECONDARY);
    const ImVec4 actived_col = color(COLORS::MAIN);

    auto col_builder = AnimBuilder<ImVec4>().range(default_col, actived_col).duration(milliseconds(250)).condition(hovered).play_policy(PlayPolicy::ENDLESS);
    const ImVec4 col = col_builder.build(col_anim_name, ImGui::GetIO().DeltaTime);

    // Animate Rotate
    std::string rotate_anim_name = "SocialButton/Rotate#"s + icon;
    auto rotate_builder = AnimBuilder<float>().range(90.f, -270.f).easing(Easing::QuadraticEaseInOut).duration(milliseconds(500)).condition(hovered).play_policy(PlayPolicy::ENDLESS);
    float rotation = rotate_builder.build(rotate_anim_name, ImGui::GetIO().DeltaTime);

    ImGui::PushStyleColor(ImGuiCol_Text, col);

    ImRotations::Start();
    ImGui::RenderTextClipped(bb.Min, bb.Max, icon, NULL, &icon_size, style.ButtonTextAlign, &bb);
    ImRotations::End(ImRotations::deg2rad(rotation));

    ImGui::PopStyleColor();

    if (hovered)
        g.MouseCursor = ImGuiMouseCursor_Hand;

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
    return pressed;
}

bool Menu::Selectable(const char* label, ImTextureID user_texture_id, bool selected, ImGuiSelectableFlags flags, const ImVec2& size_arg)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const float& alpha = style.Alpha;

    // Submit label or explicit size to ItemSize(), whereas ItemAdd() will submit a larger/spanning rectangle.
    ImGuiID id = window->GetID(label);
    ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    ImVec2 size(size_arg.x != 0.0f ? size_arg.x : label_size.x, size_arg.y != 0.0f ? size_arg.y : label_size.y);
    ImVec2 pos = window->DC.CursorPos;
    pos.y += window->DC.CurrLineTextBaseOffset;
    ImGui::ItemSize(size, 0.0f);

    // Fill horizontal space
    // We don't support (size < 0.0f) in Selectable() because the ItemSpacing extension would make explicitly right-aligned sizes not visibly match other widgets.
    const bool span_all_columns = (flags & ImGuiSelectableFlags_SpanAllColumns) != 0;
    const float min_x = span_all_columns ? window->ParentWorkRect.Min.x : pos.x;
    const float max_x = span_all_columns ? window->ParentWorkRect.Max.x : window->WorkRect.Max.x;
    if (size_arg.x == 0.0f || (flags & ImGuiSelectableFlags_SpanAvailWidth))
        size.x = ImMax(label_size.x, max_x - min_x);

    // Text stays at the submission position, but bounding box may be extended on both sides
    const float image_size = size.y;
    const ImVec2 text_min = pos + ImVec2(image_size + 4.f, 0);
    const ImVec2 text_max(min_x + size.x, pos.y + size.y);

    // Selectables are meant to be tightly packed together with no click-gap, so we extend their box to cover spacing between selectable.
    // FIXME: Not part of layout so not included in clipper calculation, but ItemSize currently doesn't allow offsetting CursorPos.
    ImRect bb(min_x, pos.y, text_max.x, text_max.y);
    if ((flags & ImGuiSelectableFlags_NoPadWithHalfSpacing) == 0)
    {
        const float spacing_x = span_all_columns ? 0.0f : style.ItemSpacing.x;
        const float spacing_y = style.ItemSpacing.y;
        const float spacing_L = IM_TRUNC(spacing_x * 0.50f);
        const float spacing_U = IM_TRUNC(spacing_y * 0.50f);
        bb.Min.x -= spacing_L;
        bb.Min.y -= spacing_U;
        bb.Max.x += (spacing_x - spacing_L);
        bb.Max.y += (spacing_y - spacing_U);
    }
    //if (g.IO.KeyCtrl) { GetForegroundDrawList()->AddRect(bb.Min, bb.Max, IM_COL32(0, 255, 0, 255)); }

    const bool disabled_item = (flags & ImGuiSelectableFlags_Disabled) != 0;
    const ImGuiItemFlags extra_item_flags = disabled_item ? (ImGuiItemFlags)ImGuiItemFlags_Disabled : ImGuiItemFlags_None;
    bool is_visible;
    if (span_all_columns)
    {
        // Modify ClipRect for the ItemAdd(), faster than doing a PushColumnsBackground/PushTableBackgroundChannel for every Selectable..
        const float backup_clip_rect_min_x = window->ClipRect.Min.x;
        const float backup_clip_rect_max_x = window->ClipRect.Max.x;
        window->ClipRect.Min.x = window->ParentWorkRect.Min.x;
        window->ClipRect.Max.x = window->ParentWorkRect.Max.x;
        is_visible = ImGui::ItemAdd(bb, id, NULL, extra_item_flags);
        window->ClipRect.Min.x = backup_clip_rect_min_x;
        window->ClipRect.Max.x = backup_clip_rect_max_x;
    }
    else
    {
        is_visible = ImGui::ItemAdd(bb, id, NULL, extra_item_flags);
    }

    const bool is_multi_select = (g.LastItemData.ItemFlags & ImGuiItemFlags_IsMultiSelect) != 0;
    if (!is_visible)
        if (!is_multi_select || !g.BoxSelectState.UnclipMode || !g.BoxSelectState.UnclipRect.Overlaps(bb)) // Extra layer of "no logic clip" for box-select support (would be more overhead to add to ItemAdd)
            return false;

    const bool disabled_global = (g.CurrentItemFlags & ImGuiItemFlags_Disabled) != 0;
    if (disabled_item && !disabled_global) // Only testing this as an optimization
        ImGui::BeginDisabled();

    // FIXME: We can standardize the behavior of those two, we could also keep the fast path of override ClipRect + full push on render only,
    // which would be advantageous since most selectable are not selected.
    if (span_all_columns)
    {
        if (g.CurrentTable)
            ImGui::TablePushBackgroundChannel();
        else if (window->DC.CurrentColumns)
            ImGui::PushColumnsBackground();
        g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_HasClipRect;
        g.LastItemData.ClipRect = window->ClipRect;
    }

    // We use NoHoldingActiveID on menus so user can click and _hold_ on a menu then drag to browse child entries
    ImGuiButtonFlags button_flags = 0;
    if (flags & ImGuiSelectableFlags_NoHoldingActiveID) { button_flags |= ImGuiButtonFlags_NoHoldingActiveId; }
    if (flags & ImGuiSelectableFlags_NoSetKeyOwner) { button_flags |= ImGuiButtonFlags_NoSetKeyOwner; }
    if (flags & ImGuiSelectableFlags_SelectOnClick) { button_flags |= ImGuiButtonFlags_PressedOnClick; }
    if (flags & ImGuiSelectableFlags_SelectOnRelease) { button_flags |= ImGuiButtonFlags_PressedOnRelease; }
    if (flags & ImGuiSelectableFlags_AllowDoubleClick) { button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick; }
    if ((flags & ImGuiSelectableFlags_AllowOverlap) || (g.LastItemData.ItemFlags & ImGuiItemFlags_AllowOverlap)) { button_flags |= ImGuiButtonFlags_AllowOverlap; }

    // Multi-selection support (header)
    const bool was_selected = selected;
    if (is_multi_select)
    {
        // Handle multi-select + alter button flags for it
        ImGui::MultiSelectItemHeader(id, &selected, &button_flags);
    }

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, button_flags);

    if ((flags & ImGuiSelectableFlags_SelectOnNav) && g.NavJustMovedToId != 0 && g.NavJustMovedToFocusScopeId == g.CurrentFocusScopeId)
        if (g.NavJustMovedToId == id)
            selected = pressed = true;

    // Update NavId when clicking or when Hovering (this doesn't happen on most widgets), so navigation can be resumed with keyboard/gamepad
    if (pressed || (hovered && (flags & ImGuiSelectableFlags_SetNavIdOnHover)))
    {
        if (!g.NavHighlightItemUnderNav && g.NavWindow == window && g.NavLayer == window->DC.NavLayerCurrent)
        {
            ImGui::SetNavID(id, window->DC.NavLayerCurrent, g.CurrentFocusScopeId, ImGui::WindowRectAbsToRel(window, bb)); // (bb == NavRect)
            if (g.IO.ConfigNavCursorVisibleAuto)
                g.NavCursorVisible = false;
        }
    }
    if (pressed)
        ImGui::MarkItemEdited(id);

    if (selected != was_selected)
        g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_ToggledSelection;

    // Render
    if (is_visible)
    {
        if (g.NavId == id)
        {
            ImGuiNavRenderCursorFlags nav_render_cursor_flags = ImGuiNavRenderCursorFlags_Compact | ImGuiNavRenderCursorFlags_NoRounding;
            if (is_multi_select)
                nav_render_cursor_flags |= ImGuiNavRenderCursorFlags_AlwaysDraw; // Always show the nav rectangle
            ImGui::RenderNavCursor(bb, id, nav_render_cursor_flags);
        }
    }

    if (span_all_columns)
    {
        if (g.CurrentTable)
            ImGui::TablePopBackgroundChannel();
        else if (window->DC.CurrentColumns)
            ImGui::PopColumnsBackground();
    }

    if (is_visible) {
        const std::string color_anim_name = "Selectable/Color#"s + label;
        const ImVec4 default_col = color(COLORS::WHITE);
        const ImVec4 actived_col = color(COLORS::MAIN);

        auto color_builder = AnimBuilder<ImVec4>().range(default_col, actived_col).duration(milliseconds(150)).easing(Easing::QuadraticEaseIn).condition(selected).play_policy(PlayPolicy::ENDLESS);
        ImVec4 color = color_builder.build(color_anim_name, ImGui::GetIO().DeltaTime);

        const ImVec2 image_pos(bb.Min.x, bb.GetCenter().y - image_size * 0.5f);
        window->DrawList->AddImage(user_texture_id, image_pos, image_pos + ImVec2(image_size, image_size), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, static_cast<int>(255.f * style.Alpha)));

        color.w = style.Alpha;
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::RenderTextClipped(text_min, text_max, label, NULL, &label_size, ImVec2(0.f, 0.5f), &bb);
        ImGui::PopStyleColor();
    }

    // Automatically close popups
    if (pressed && !(flags & ImGuiSelectableFlags_NoAutoClosePopups) && (g.LastItemData.ItemFlags & ImGuiItemFlags_AutoClosePopups))
        CloseCurrentPopupAnimated();

    if (disabled_item && !disabled_global)
        ImGui::EndDisabled();

    if (hovered)
        g.MouseCursor = ImGuiMouseCursor_Hand;

    // Selectable() always returns a pressed state!
    // Users of BeginMultiSelect()/EndMultiSelect() scope: you may call ImGui::IsItemToggledSelection() to retrieve
    // selection toggle, only useful if you need that state updated (e.g. for rendering purpose) before reaching EndMultiSelect().
    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
    return pressed; //-V1020
}

bool Menu::Combo(const char* label, int& current_item, std::initializer_list<const char*> items, std::initializer_list<ImTextureID> images) {
    ImGuiContext& g = *GImGui;

    std::string preview_value;
    ImTextureID preview_image = 0;
    const size_t label_hash = hash(label);
    if (current_item >= 0 && current_item < items.size()) {
        preview_value = animate_string(label_hash, *(items.begin() + current_item), ImGui::GetIO().DeltaTime);
        preview_image = *(images.begin() + current_item);
    }

    bool value_changed = false;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 8));
    ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 12.f);
    if (Menu::BeginCombo(label, preview_value.c_str(), preview_image, items.size()))
    {
        for (int i = 0; i < items.size(); i++)
        {
            const bool is_selected = (current_item == i);
            const std::string item_text = std::string(*(items.begin() + i)) + "##" + label;
            const ImTextureID item_image = *(images.begin() + i);
            if (Menu::Selectable(item_text.c_str(), item_image, is_selected, 0, ImVec2(0.f, 20.f))) {
                current_item = i;
                value_changed = true;
            }
            if (is_selected)    
                ImGui::SetItemDefaultFocus();

        }
        Menu::EndCombo(label);
    }
    ImGui::PopStyleVar(2);

    if (value_changed) {
        auto& locals_data = get_locals_data();
        auto& locals_time = get_locals_time();
        locals_data.erase(label_hash);
        locals_time.erase(label_hash);
        ImGui::MarkItemEdited(g.LastItemData.ID);
    }

    return value_changed;
}

bool Menu::BeginCombo(const char* label, const char* preview_value, ImTextureID preview_image, size_t items_count, ImGuiComboFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = ImGui::GetCurrentWindow();

    ImGuiNextWindowDataFlags backup_next_window_data_flags = g.NextWindowData.Flags;
    g.NextWindowData.ClearFlags(); // We behave like Begin() and need to consume those values
    if (window->SkipItems)
        return false;

    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    const float arrow_width = 26.f;
    const float preview_width = ((flags & ImGuiComboFlags_WidthFitPreview) && (preview_value != NULL)) ? ImGui::CalcTextSize(preview_value, NULL, true).x : 0.0f;
    const float w = (flags & ImGuiComboFlags_NoPreview) ? 0.f : ((flags & ImGuiComboFlags_WidthFitPreview) ? (preview_width + style.FramePadding.x * 2.0f) : ImGui::CalcItemWidth());

    const ImVec2 pos = window->DC.CursorPos;
    const ImRect bb(pos, pos + ImVec2(170, 32));
    ImGui::ItemSize(bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id, &bb))
        return false;

    // Open on click
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    bool popup_open = ImHelpers::IsPopupAnimatedOpen(label);

    // Render shape
    window->DrawList->AddRectFilled(bb.Min, bb.Max, u32color(COLORS::FOREGROUND), style.FrameRounding, ImDrawFlags_RoundCornersAll);

    const std::string color_anim_name = "BeginCombo/Color#"s + label;
    bool anim_condition = hovered || popup_open;
    auto color_builder = AnimBuilder<ImVec4>().range(color(COLORS::WHITE), color(COLORS::MAIN)).condition(anim_condition).play_policy(PlayPolicy::ENDLESS);
    ImVec4 col = color_builder.build(color_anim_name, ImGui::GetIO().DeltaTime);
    col.w = style.Alpha;

    const std::string rotate_anim_name = "BeginCombo/Rotate#"s + label;
    auto rotate_builder = AnimBuilder<float>().range(0.f ,1.f).condition(popup_open).duration(milliseconds(300)).easing(Easing::QuadraticEaseInOut).play_policy(PlayPolicy::ENDLESS);
    const float rotate = rotate_builder.build(rotate_anim_name, ImGui::GetIO().DeltaTime);

    const ImRect arrow_bb(ImVec2(bb.Max.x - arrow_width, bb.Min.y), bb.Max);
    ImGui::PushStyleColor(ImGuiCol_Text, col);
    ImRotations::Start();
    ImGui::RenderTextClipped(arrow_bb.Min + ImVec2(0.f, 3.f), arrow_bb.Max, ICON_FA_ANGLE_UP, NULL, NULL, style.ButtonTextAlign, &arrow_bb);
    ImRotations::End(ImRotations::deg2rad(90.f + rotate * 180.f));
    ImGui::PopStyleColor();

    if (hovered)
        g.MouseCursor = ImGuiMouseCursor_Hand;

    // Render preview and label
    if (preview_value != NULL && !(flags & ImGuiComboFlags_NoPreview))
    {
        if (g.LogEnabled)
            ImGui::LogSetNextTextDecoration("{", "}");


        const float image_size = 20.f;
        const ImVec2 image_pos(bb.Min.x + 8.f, bb.GetCenter().y - 20.f * 0.5f);
        const ImRect text_rect(bb.Min + ImVec2(image_size + 16.f, 0), ImVec2(bb.Max.x - arrow_width, bb.Max.y));
        window->DrawList->AddImage(preview_image, image_pos, image_pos + ImVec2(image_size, image_size), ImVec2(0,0), ImVec2(1,1), ImColor(255, 255, 255, static_cast<int>(255.f * style.Alpha)));

        ImGui::PushStyleColor(ImGuiCol_Text, col);
        ImGui::RenderTextClipped(text_rect.Min, text_rect.Max, preview_value, NULL, NULL, ImVec2(0.0f, 0.5f), &text_rect);
        ImGui::PopStyleColor();
    }

    g.NextWindowData.Flags = backup_next_window_data_flags;

    ImGui::PushStyleColor(ImGuiCol_PopupBg, color(COLORS::FOREGROUND));
    const ImVec2 popup_size(bb.GetWidth(), style.WindowPadding.y * 2.f + 20.f * items_count + style.ItemSpacing.y * (items_count - 1));
    bool ret = ImHelpers::BeginPopupAnimated(label, pressed, bb.GetBL() - ImVec2(0, bb.GetHeight() + popup_size.y), popup_size, ImHelpers::PopupAnimStyles::BOTTOM);
    ImGui::PopStyleColor();
    current_popup_animated_id = label;

    return ret;
}
void Menu::EndCombo(const char* str_id)
{
    ImHelpers::EndPopupAnimated(str_id);
    current_popup_animated_id = nullptr;
}

// For InputTextEx()
static bool     InputTextFilterCharacter(ImGuiContext* ctx, unsigned int* p_char, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data, bool input_source_is_clipboard = false);
static int      InputTextCalcTextLenAndLineCount(const char* text_begin, const char** out_text_end);
static ImVec2   InputTextCalcTextSize(ImGuiContext* ctx, const char* text_begin, const char* text_end, const char** remaining = NULL, ImVec2* out_offset = NULL, bool stop_on_new_line = false);

namespace ImStb
{
#include <imgui/imstb_textedit.h>
}

// This is only used in the path where the multiline widget is inactivate.
static int InputTextCalcTextLenAndLineCount(const char* text_begin, const char** out_text_end)
{
    int line_count = 0;
    const char* s = text_begin;
    while (true)
    {
        const char* s_eol = strchr(s, '\n');
        line_count++;
        if (s_eol == NULL)
        {
            s = s + strlen(s);
            break;
        }
        s = s_eol + 1;
    }
    *out_text_end = s;
    return line_count;
}

// FIXME: Ideally we'd share code with ImFont::CalcTextSizeA()
static ImVec2 InputTextCalcTextSize(ImGuiContext* ctx, const char* text_begin, const char* text_end, const char** remaining, ImVec2* out_offset, bool stop_on_new_line)
{
    ImGuiContext& g = *ctx;
    ImFont* font = g.Font;
    const float line_height = g.FontSize;
    const float scale = line_height / font->FontSize;

    ImVec2 text_size = ImVec2(0, 0);
    float line_width = 0.0f;

    const char* s = text_begin;
    while (s < text_end)
    {
        unsigned int c = (unsigned int)*s;
        if (c < 0x80)
            s += 1;
        else
            s += ImTextCharFromUtf8(&c, s, text_end);

        if (c == '\n')
        {
            text_size.x = ImMax(text_size.x, line_width);
            text_size.y += line_height;
            line_width = 0.0f;
            if (stop_on_new_line)
                break;
            continue;
        }
        if (c == '\r')
            continue;

        const float char_width = ((int)c < font->IndexAdvanceX.Size ? font->IndexAdvanceX.Data[c] : font->FallbackAdvanceX) * scale;
        line_width += char_width;
    }

    if (text_size.x < line_width)
        text_size.x = line_width;

    if (out_offset)
        *out_offset = ImVec2(line_width, text_size.y + line_height);  // offset allow for the possibility of sitting after a trailing \n

    if (line_width > 0 || text_size.y == 0.0f)                        // whereas size.y will ignore the trailing \n
        text_size.y += line_height;

    if (remaining)
        *remaining = s;

    return text_size;
}

// Wrapper for stb_textedit.h to edit text (our wrapper is for: statically sized buffer, single-line, wchar characters. InputText converts between UTF-8 and wchar)
// With our UTF-8 use of stb_textedit:
// - STB_TEXTEDIT_GETCHAR is nothing more than a a "GETBYTE". It's only used to compare to ascii or to copy blocks of text so we are fine.
// - One exception is the STB_TEXTEDIT_IS_SPACE feature which would expect a full char in order to handle full-width space such as 0x3000 (see ImCharIsBlankW).
// - ...but we don't use that feature.
namespace ImStb
{
    static int     STB_TEXTEDIT_STRINGLEN(const ImGuiInputTextState* obj) { return obj->TextLen; }
    static char    STB_TEXTEDIT_GETCHAR(const ImGuiInputTextState* obj, int idx) { IM_ASSERT(idx <= obj->TextLen); return obj->TextA[idx]; }
    static float   STB_TEXTEDIT_GETWIDTH(ImGuiInputTextState* obj, int line_start_idx, int char_idx) { unsigned int c; ImTextCharFromUtf8(&c, obj->TextA.Data + line_start_idx + char_idx, obj->TextA.Data + obj->TextLen); if ((ImWchar)c == '\n') return IMSTB_TEXTEDIT_GETWIDTH_NEWLINE; ImGuiContext& g = *obj->Ctx; return g.Font->GetCharAdvance((ImWchar)c) * g.FontScale; }
    static char    STB_TEXTEDIT_NEWLINE = '\n';
    static void    STB_TEXTEDIT_LAYOUTROW(StbTexteditRow* r, ImGuiInputTextState* obj, int line_start_idx)
    {
        const char* text = obj->TextA.Data;
        const char* text_remaining = NULL;
        const ImVec2 size = InputTextCalcTextSize(obj->Ctx, text + line_start_idx, text + obj->TextLen, &text_remaining, NULL, true);
        r->x0 = 0.0f;
        r->x1 = size.x;
        r->baseline_y_delta = size.y;
        r->ymin = 0.0f;
        r->ymax = size.y;
        r->num_chars = (int)(text_remaining - (text + line_start_idx));
    }

#define IMSTB_TEXTEDIT_GETNEXTCHARINDEX  IMSTB_TEXTEDIT_GETNEXTCHARINDEX_IMPL
#define IMSTB_TEXTEDIT_GETPREVCHARINDEX  IMSTB_TEXTEDIT_GETPREVCHARINDEX_IMPL

    static int IMSTB_TEXTEDIT_GETNEXTCHARINDEX_IMPL(ImGuiInputTextState* obj, int idx)
    {
        if (idx >= obj->TextLen)
            return obj->TextLen + 1;
        unsigned int c;
        return idx + ImTextCharFromUtf8(&c, obj->TextA.Data + idx, obj->TextA.Data + obj->TextLen);
    }

    static int IMSTB_TEXTEDIT_GETPREVCHARINDEX_IMPL(ImGuiInputTextState* obj, int idx)
    {
        if (idx <= 0)
            return -1;
        const char* p = ImTextFindPreviousUtf8Codepoint(obj->TextA.Data, obj->TextA.Data + idx);
        return (int)(p - obj->TextA.Data);
    }

    static bool ImCharIsSeparatorW(unsigned int c)
    {
        static const unsigned int separator_list[] =
        {
            ',', 0x3001, '.', 0x3002, ';', 0xFF1B, '(', 0xFF08, ')', 0xFF09, '{', 0xFF5B, '}', 0xFF5D,
            '[', 0x300C, ']', 0x300D, '|', 0xFF5C, '!', 0xFF01, '\\', 0xFFE5, '/', 0x30FB, 0xFF0F,
            '\n', '\r',
        };
        for (unsigned int separator : separator_list)
            if (c == separator)
                return true;
        return false;
    }

    static int is_word_boundary_from_right(ImGuiInputTextState* obj, int idx)
    {
        // When ImGuiInputTextFlags_Password is set, we don't want actions such as CTRL+Arrow to leak the fact that underlying data are blanks or separators.
        if ((obj->Flags & ImGuiInputTextFlags_Password) || idx <= 0)
            return 0;

        const char* curr_p = obj->TextA.Data + idx;
        const char* prev_p = ImTextFindPreviousUtf8Codepoint(obj->TextA.Data, curr_p);
        unsigned int curr_c; ImTextCharFromUtf8(&curr_c, curr_p, obj->TextA.Data + obj->TextLen);
        unsigned int prev_c; ImTextCharFromUtf8(&prev_c, prev_p, obj->TextA.Data + obj->TextLen);

        bool prev_white = ImCharIsBlankW(prev_c);
        bool prev_separ = ImCharIsSeparatorW(prev_c);
        bool curr_white = ImCharIsBlankW(curr_c);
        bool curr_separ = ImCharIsSeparatorW(curr_c);
        return ((prev_white || prev_separ) && !(curr_separ || curr_white)) || (curr_separ && !prev_separ);
    }
    static int is_word_boundary_from_left(ImGuiInputTextState* obj, int idx)
    {
        if ((obj->Flags & ImGuiInputTextFlags_Password) || idx <= 0)
            return 0;

        const char* curr_p = obj->TextA.Data + idx;
        const char* prev_p = ImTextFindPreviousUtf8Codepoint(obj->TextA.Data, curr_p);
        unsigned int prev_c; ImTextCharFromUtf8(&prev_c, curr_p, obj->TextA.Data + obj->TextLen);
        unsigned int curr_c; ImTextCharFromUtf8(&curr_c, prev_p, obj->TextA.Data + obj->TextLen);

        bool prev_white = ImCharIsBlankW(prev_c);
        bool prev_separ = ImCharIsSeparatorW(prev_c);
        bool curr_white = ImCharIsBlankW(curr_c);
        bool curr_separ = ImCharIsSeparatorW(curr_c);
        return ((prev_white) && !(curr_separ || curr_white)) || (curr_separ && !prev_separ);
    }
    static int  STB_TEXTEDIT_MOVEWORDLEFT_IMPL(ImGuiInputTextState* obj, int idx)
    {
        idx = IMSTB_TEXTEDIT_GETPREVCHARINDEX(obj, idx);
        while (idx >= 0 && !is_word_boundary_from_right(obj, idx))
            idx = IMSTB_TEXTEDIT_GETPREVCHARINDEX(obj, idx);
        return idx < 0 ? 0 : idx;
    }
    static int  STB_TEXTEDIT_MOVEWORDRIGHT_MAC(ImGuiInputTextState* obj, int idx)
    {
        int len = obj->TextLen;
        idx = IMSTB_TEXTEDIT_GETNEXTCHARINDEX(obj, idx);
        while (idx < len && !is_word_boundary_from_left(obj, idx))
            idx = IMSTB_TEXTEDIT_GETNEXTCHARINDEX(obj, idx);
        return idx > len ? len : idx;
    }
    static int  STB_TEXTEDIT_MOVEWORDRIGHT_WIN(ImGuiInputTextState* obj, int idx)
    {
        idx = IMSTB_TEXTEDIT_GETNEXTCHARINDEX(obj, idx);
        int len = obj->TextLen;
        while (idx < len && !is_word_boundary_from_right(obj, idx))
            idx = IMSTB_TEXTEDIT_GETNEXTCHARINDEX(obj, idx);
        return idx > len ? len : idx;
    }
    static int  STB_TEXTEDIT_MOVEWORDRIGHT_IMPL(ImGuiInputTextState* obj, int idx) { ImGuiContext& g = *obj->Ctx; if (g.IO.ConfigMacOSXBehaviors) return STB_TEXTEDIT_MOVEWORDRIGHT_MAC(obj, idx); else return STB_TEXTEDIT_MOVEWORDRIGHT_WIN(obj, idx); }
#define STB_TEXTEDIT_MOVEWORDLEFT       STB_TEXTEDIT_MOVEWORDLEFT_IMPL  // They need to be #define for stb_textedit.h
#define STB_TEXTEDIT_MOVEWORDRIGHT      STB_TEXTEDIT_MOVEWORDRIGHT_IMPL

    static void STB_TEXTEDIT_DELETECHARS(ImGuiInputTextState* obj, int pos, int n)
    {
        char* dst = obj->TextA.Data + pos;

        obj->Edited = true;
        obj->TextLen -= n;

        // Offset remaining text (FIXME-OPT: Use memmove)
        const char* src = obj->TextA.Data + pos + n;
        while (char c = *src++)
            *dst++ = c;
        *dst = '\0';
    }

    static bool STB_TEXTEDIT_INSERTCHARS(ImGuiInputTextState* obj, int pos, const char* new_text, int new_text_len)
    {
        const bool is_resizable = (obj->Flags & ImGuiInputTextFlags_CallbackResize) != 0;
        const int text_len = obj->TextLen;
        IM_ASSERT(pos <= text_len);

        if (!is_resizable && (new_text_len + obj->TextLen + 1 > obj->BufCapacity))
            return false;

        // Grow internal buffer if needed
        if (new_text_len + text_len + 1 > obj->TextA.Size)
        {
            if (!is_resizable)
                return false;
            obj->TextA.resize(text_len + ImClamp(new_text_len, 32, ImMax(256, new_text_len)) + 1);
        }

        char* text = obj->TextA.Data;
        if (pos != text_len)
            memmove(text + pos + new_text_len, text + pos, (size_t)(text_len - pos));
        memcpy(text + pos, new_text, (size_t)new_text_len);

        obj->Edited = true;
        obj->TextLen += new_text_len;
        obj->TextA[obj->TextLen] = '\0';

        return true;
    }

    // We don't use an enum so we can build even with conflicting symbols (if another user of stb_textedit.h leak their STB_TEXTEDIT_K_* symbols)
#define STB_TEXTEDIT_K_LEFT         0x200000 // keyboard input to move cursor left
#define STB_TEXTEDIT_K_RIGHT        0x200001 // keyboard input to move cursor right
#define STB_TEXTEDIT_K_UP           0x200002 // keyboard input to move cursor up
#define STB_TEXTEDIT_K_DOWN         0x200003 // keyboard input to move cursor down
#define STB_TEXTEDIT_K_LINESTART    0x200004 // keyboard input to move cursor to start of line
#define STB_TEXTEDIT_K_LINEEND      0x200005 // keyboard input to move cursor to end of line
#define STB_TEXTEDIT_K_TEXTSTART    0x200006 // keyboard input to move cursor to start of text
#define STB_TEXTEDIT_K_TEXTEND      0x200007 // keyboard input to move cursor to end of text
#define STB_TEXTEDIT_K_DELETE       0x200008 // keyboard input to delete selection or character under cursor
#define STB_TEXTEDIT_K_BACKSPACE    0x200009 // keyboard input to delete selection or character left of cursor
#define STB_TEXTEDIT_K_UNDO         0x20000A // keyboard input to perform undo
#define STB_TEXTEDIT_K_REDO         0x20000B // keyboard input to perform redo
#define STB_TEXTEDIT_K_WORDLEFT     0x20000C // keyboard input to move cursor left one word
#define STB_TEXTEDIT_K_WORDRIGHT    0x20000D // keyboard input to move cursor right one word
#define STB_TEXTEDIT_K_PGUP         0x20000E // keyboard input to move cursor up a page
#define STB_TEXTEDIT_K_PGDOWN       0x20000F // keyboard input to move cursor down a page
#define STB_TEXTEDIT_K_SHIFT        0x400000

#define IMSTB_TEXTEDIT_IMPLEMENTATION
#define IMSTB_TEXTEDIT_memmove memmove
#include <imgui/imstb_textedit.h>

// stb_textedit internally allows for a single undo record to do addition and deletion, but somehow, calling
// the stb_textedit_paste() function creates two separate records, so we perform it manually. (FIXME: Report to nothings/stb?)
    static void stb_textedit_replace(ImGuiInputTextState* str, STB_TexteditState* state, const IMSTB_TEXTEDIT_CHARTYPE* text, int text_len)
    {
        stb_text_makeundo_replace(str, state, 0, str->TextLen, text_len);
        ImStb::STB_TEXTEDIT_DELETECHARS(str, 0, str->TextLen);
        state->cursor = state->select_start = state->select_end = 0;
        if (text_len <= 0)
            return;
        if (ImStb::STB_TEXTEDIT_INSERTCHARS(str, 0, text, text_len))
        {
            state->cursor = state->select_start = state->select_end = text_len;
            state->has_preferred_x = 0;
            return;
        }
        IM_ASSERT(0); // Failed to insert character, normally shouldn't happen because of how we currently use stb_textedit_replace()
    }

} // namespace ImStb
// Return false to discard a character.
static bool InputTextFilterCharacter(ImGuiContext* ctx, unsigned int* p_char, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data, bool input_source_is_clipboard)
{

    unsigned int c = *p_char;

    // Filter non-printable (NB: isprint is unreliable! see #2467)
    bool apply_named_filters = true;
    if (c < 0x20)
    {
        bool pass = false;
        pass |= (c == '\n') && (flags & ImGuiInputTextFlags_Multiline) != 0; // Note that an Enter KEY will emit \r and be ignored (we poll for KEY in InputText() code)
        pass |= (c == '\t') && (flags & ImGuiInputTextFlags_AllowTabInput) != 0;
        if (!pass)
            return false;
        apply_named_filters = false; // Override named filters below so newline and tabs can still be inserted.
    }

    if (input_source_is_clipboard == false)
    {
        // We ignore Ascii representation of delete (emitted from Backspace on OSX, see #2578, #2817)
        if (c == 127)
            return false;

        // Filter private Unicode range. GLFW on OSX seems to send private characters for special keys like arrow keys (FIXME)
        if (c >= 0xE000 && c <= 0xF8FF)
            return false;
    }

    // Filter Unicode ranges we are not handling in this build
    if (c > IM_UNICODE_CODEPOINT_MAX)
        return false;

    // Generic named filters
    if (apply_named_filters && (flags & (ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_CharsScientific | (ImGuiInputTextFlags)ImGuiInputTextFlags_LocalizeDecimalPoint)))
    {
        // The libc allows overriding locale, with e.g. 'setlocale(LC_NUMERIC, "de_DE.UTF-8");' which affect the output/input of printf/scanf to use e.g. ',' instead of '.'.
        // The standard mandate that programs starts in the "C" locale where the decimal point is '.'.
        // We don't really intend to provide widespread support for it, but out of empathy for people stuck with using odd API, we support the bare minimum aka overriding the decimal point.
        // Change the default decimal_point with:
        //   ImGui::GetPlatformIO()->Platform_LocaleDecimalPoint = *localeconv()->decimal_point;
        // Users of non-default decimal point (in particular ',') may be affected by word-selection logic (is_word_boundary_from_right/is_word_boundary_from_left) functions.
        ImGuiContext& g = *ctx;
        const unsigned c_decimal_point = (unsigned int)g.PlatformIO.Platform_LocaleDecimalPoint;
        if (flags & (ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CharsScientific | (ImGuiInputTextFlags)ImGuiInputTextFlags_LocalizeDecimalPoint))
            if (c == '.' || c == ',')
                c = c_decimal_point;

        // Full-width -> half-width conversion for numeric fields (https://en.wikipedia.org/wiki/Halfwidth_and_Fullwidth_Forms_(Unicode_block)
        // While this is mostly convenient, this has the side-effect for uninformed users accidentally inputting full-width characters that they may
        // scratch their head as to why it works in numerical fields vs in generic text fields it would require support in the font.
        if (flags & (ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CharsScientific | ImGuiInputTextFlags_CharsHexadecimal))
            if (c >= 0xFF01 && c <= 0xFF5E)
                c = c - 0xFF01 + 0x21;

        // Allow 0-9 . - + * /
        if (flags & ImGuiInputTextFlags_CharsDecimal)
            if (!(c >= '0' && c <= '9') && (c != c_decimal_point) && (c != '-') && (c != '+') && (c != '*') && (c != '/'))
                return false;

        // Allow 0-9 . - + * / e E
        if (flags & ImGuiInputTextFlags_CharsScientific)
            if (!(c >= '0' && c <= '9') && (c != c_decimal_point) && (c != '-') && (c != '+') && (c != '*') && (c != '/') && (c != 'e') && (c != 'E'))
                return false;

        // Allow 0-9 a-F A-F
        if (flags & ImGuiInputTextFlags_CharsHexadecimal)
            if (!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'f') && !(c >= 'A' && c <= 'F'))
                return false;

        // Turn a-z into A-Z
        if (flags & ImGuiInputTextFlags_CharsUppercase)
            if (c >= 'a' && c <= 'z')
                c += (unsigned int)('A' - 'a');

        if (flags & ImGuiInputTextFlags_CharsNoBlank)
            if (ImCharIsBlankW(c))
                return false;

        *p_char = c;
    }

    // Custom callback filter
    if (flags & ImGuiInputTextFlags_CallbackCharFilter)
    {
        ImGuiContext& g = *GImGui;
        ImGuiInputTextCallbackData callback_data;
        callback_data.Ctx = &g;
        callback_data.EventFlag = ImGuiInputTextFlags_CallbackCharFilter;
        callback_data.EventChar = (ImWchar)c;
        callback_data.Flags = flags;
        callback_data.UserData = user_data;
        if (callback(&callback_data) != 0)
            return false;
        *p_char = callback_data.EventChar;
        if (!callback_data.EventChar)
            return false;
    }

    return true;
}

// Find the shortest single replacement we can make to get the new text from the old text.
// Important: needs to be run before TextW is rewritten with the new characters because calling STB_TEXTEDIT_GETCHAR() at the end.
// FIXME: Ideally we should transition toward (1) making InsertChars()/DeleteChars() update undo-stack (2) discourage (and keep reconcile) or obsolete (and remove reconcile) accessing buffer directly.
static void InputTextReconcileUndoStateAfterUserCallback(ImGuiInputTextState* state, const char* new_buf_a, int new_length_a)
{
    const char* old_buf = state->CallbackTextBackup.Data;
    const int old_length = state->CallbackTextBackup.Size - 1;

    const int shorter_length = ImMin(old_length, new_length_a);
    int first_diff;
    for (first_diff = 0; first_diff < shorter_length; first_diff++)
        if (old_buf[first_diff] != new_buf_a[first_diff])
            break;
    if (first_diff == old_length && first_diff == new_length_a)
        return;

    int old_last_diff = old_length - 1;
    int new_last_diff = new_length_a - 1;
    for (; old_last_diff >= first_diff && new_last_diff >= first_diff; old_last_diff--, new_last_diff--)
        if (old_buf[old_last_diff] != new_buf_a[new_last_diff])
            break;

    const int insert_len = new_last_diff - first_diff + 1;
    const int delete_len = old_last_diff - first_diff + 1;
    if (insert_len > 0 || delete_len > 0)
        if (IMSTB_TEXTEDIT_CHARTYPE* p = stb_text_createundo(&state->Stb->undostate, first_diff, delete_len, insert_len))
            for (int i = 0; i < delete_len; i++)
                p[i] = old_buf[first_diff + i];
}

// Edit a string of text
// - buf_size account for the zero-terminator, so a buf_size of 6 can hold "Hello" but not "Hello!".
//   This is so we can easily call InputText() on static arrays using ARRAYSIZE() and to match
//   Note that in std::string world, capacity() would omit 1 byte used by the zero-terminator.
// - When active, hold on a privately held copy of the text (and apply back to 'buf'). So changing 'buf' while the InputText is active has no effect.
// - If you want to use ImGui::InputText() with std::string, see misc/cpp/imgui_stdlib.h
// (FIXME: Rather confusing and messy function, among the worse part of our codebase, expecting to rewrite a V2 at some point.. Partly because we are
//  doing UTF8 > U16 > UTF8 conversions on the go to easily interface with stb_textedit. Ideally should stay in UTF-8 all the time. See https://github.com/nothings/stb/issues/188)

bool Menu::InputText(const Local& label, const char* icon, char* buf, int buf_size, const ImVec2& size_arg, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* callback_user_data)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    IM_ASSERT(buf != NULL && buf_size >= 0);
    IM_ASSERT(!((flags & ImGuiInputTextFlags_CallbackHistory) && (flags & ImGuiInputTextFlags_Multiline)));        // Can't use both together (they both use up/down keys)
    IM_ASSERT(!((flags & ImGuiInputTextFlags_CallbackCompletion) && (flags & ImGuiInputTextFlags_AllowTabInput))); // Can't use both together (they both use tab key)

    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;
    const ImGuiStyle& style = g.Style;

    const bool RENDER_SELECTION_WHEN_INACTIVE = false;
    const bool is_multiline = (flags & ImGuiInputTextFlags_Multiline) != 0;

    if (is_multiline) // Open group before calling GetID() because groups tracks id created within their scope (including the scrollbar)
        ImGui::BeginGroup();
    const ImGuiID id = window->GetID(label.id);
    const ImVec2 label_size = ImGui::CalcTextSize(label.ch(), NULL, true);

    const ImVec2 pos = window->DC.CursorPos;
    const ImVec2 size(size_arg.x != 0.0f ? size_arg.x : label_size.x, size_arg.y != 0.0f ? size_arg.y : label_size.y);

    const float icon_width = ImGui::CalcTextSize(icon).x + 21.f;

    const ImRect total_bb(pos, pos + size);
    const ImRect bb(total_bb.Min + ImVec2(style.FramePadding.x, 0), total_bb.Max - ImVec2(icon_width + style.FramePadding.x * 2.f, 0));

    ImGuiWindow* draw_window = window;

    ImVec2 inner_size = bb.GetSize();
    ImGuiLastItemData item_data_backup;
    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!(flags & ImGuiInputTextFlags_MergedItem))
        if (!ImGui::ItemAdd(total_bb, id, &bb, ImGuiItemFlags_Inputable))
            return false;

    // Ensure mouse cursor is set even after switching to keyboard/gamepad mode. May generalize further? (#6417)
    bool hovered = ImGui::ItemHoverable(total_bb, id, g.LastItemData.ItemFlags | ImGuiItemFlags_NoNavDisableMouseHover);
    if (hovered)
        ImGui::SetMouseCursor(ImGuiMouseCursor_TextInput);
    if (hovered && g.NavHighlightItemUnderNav)
        hovered = false;

    // We are only allowed to access the state if we are already the active widget.
    ImGuiInputTextState* state = ImGui::GetInputTextState(id);

    if (g.LastItemData.ItemFlags & ImGuiItemFlags_ReadOnly)
        flags |= ImGuiInputTextFlags_ReadOnly;
    const bool is_readonly = (flags & ImGuiInputTextFlags_ReadOnly) != 0;
    const bool is_password = (flags & ImGuiInputTextFlags_Password) != 0;
    const bool is_undoable = (flags & ImGuiInputTextFlags_NoUndoRedo) == 0;
    const bool is_resizable = (flags & ImGuiInputTextFlags_CallbackResize) != 0;
    if (is_resizable)
        IM_ASSERT(callback != NULL); // Must provide a callback if you set the ImGuiInputTextFlags_CallbackResize flag!

    const bool input_requested_by_nav = (g.ActiveId != id) && ((g.NavActivateId == id) && ((g.NavActivateFlags & ImGuiActivateFlags_PreferInput) || (g.NavInputSource == ImGuiInputSource_Keyboard)));

    const bool user_clicked = hovered && io.MouseClicked[0];
    const bool user_scroll_finish = is_multiline && state != NULL && g.ActiveId == 0 && g.ActiveIdPreviousFrame == ImGui::GetWindowScrollbarID(draw_window, ImGuiAxis_Y);
    const bool user_scroll_active = is_multiline && state != NULL && g.ActiveId == ImGui::GetWindowScrollbarID(draw_window, ImGuiAxis_Y);
    bool clear_active_id = false;
    bool select_all = false;

    float scroll_y = is_multiline ? draw_window->Scroll.y : FLT_MAX;

    const bool init_reload_from_user_buf = (state != NULL && state->ReloadUserBuf);
    const bool init_changed_specs = (state != NULL && state->Stb->single_line != !is_multiline); // state != NULL means its our state.
    const bool init_make_active = (user_clicked || user_scroll_finish || input_requested_by_nav);
    const bool init_state = (init_make_active || user_scroll_active);
    if ((init_state && g.ActiveId != id) || init_changed_specs || init_reload_from_user_buf)
    {
        // Access state even if we don't own it yet.
        state = &g.InputTextState;
        state->CursorAnimReset();
        state->ReloadUserBuf = false;

        // Backup state of deactivating item so they'll have a chance to do a write to output buffer on the same frame they report IsItemDeactivatedAfterEdit (#4714)
        ImGui::InputTextDeactivateHook(state->ID);

        // From the moment we focused we are normally ignoring the content of 'buf' (unless we are in read-only mode)
        const int buf_len = (int)strlen(buf);
        if (!init_reload_from_user_buf)
        {
            // Take a copy of the initial buffer value.
            state->TextToRevertTo.resize(buf_len + 1);    // UTF-8. we use +1 to make sure that .Data is always pointing to at least an empty string.
            memcpy(state->TextToRevertTo.Data, buf, buf_len + 1);
        }

        // Preserve cursor position and undo/redo stack if we come back to same widget
        // FIXME: Since we reworked this on 2022/06, may want to differentiate recycle_cursor vs recycle_undostate?
        bool recycle_state = (state->ID == id && !init_changed_specs && !init_reload_from_user_buf);
        if (recycle_state && (state->TextLen != buf_len || (strncmp(state->TextA.Data, buf, buf_len) != 0)))
            recycle_state = false;

        // Start edition
        state->ID = id;
        state->TextA.resize(buf_size + 1);          // we use +1 to make sure that .Data is always pointing to at least an empty string.
        state->TextLen = (int)strlen(buf);
        memcpy(state->TextA.Data, buf, state->TextLen + 1);

        if (recycle_state)
        {
            // Recycle existing cursor/selection/undo stack but clamp position
            // Note a single mouse click will override the cursor/position immediately by calling stb_textedit_click handler.
            state->CursorClamp();
        }
        else
        {
            state->Scroll = ImVec2(0.0f, 0.0f);
            stb_textedit_initialize_state(state->Stb, !is_multiline);
        }

        if (init_reload_from_user_buf)
        {
            state->Stb->select_start = state->ReloadSelectionStart;
            state->Stb->cursor = state->Stb->select_end = state->ReloadSelectionEnd;
            state->CursorClamp();
        }
        else if (!is_multiline)
        {
            if (flags & ImGuiInputTextFlags_AutoSelectAll)
                select_all = true;
            if (input_requested_by_nav && (!recycle_state || !(g.NavActivateFlags & ImGuiActivateFlags_TryToPreserveState)))
                select_all = true;
            if (user_clicked && io.KeyCtrl)
                select_all = true;
        }

        if (flags & ImGuiInputTextFlags_AlwaysOverwrite)
            state->Stb->insert_mode = 1; // stb field name is indeed incorrect (see #2863)
    }

    const bool is_osx = io.ConfigMacOSXBehaviors;
    if (g.ActiveId != id && init_make_active)
    {
        IM_ASSERT(state && state->ID == id);
        ImGui::SetActiveID(id, window);
        ImGui::SetFocusID(id, window);
        ImGui::FocusWindow(window);
    }
    if (g.ActiveId == id)
    {
        // Declare some inputs, the other are registered and polled via Shortcut() routing system.
        // FIXME: The reason we don't use Shortcut() is we would need a routing flag to specify multiple mods, or to all mods combinaison into individual shortcuts.
        const ImGuiKey always_owned_keys[] = { ImGuiKey_LeftArrow, ImGuiKey_RightArrow, ImGuiKey_Enter, ImGuiKey_KeypadEnter, ImGuiKey_Delete, ImGuiKey_Backspace, ImGuiKey_Home, ImGuiKey_End };
        for (ImGuiKey key : always_owned_keys)
            ImGui::SetKeyOwner(key, id);
        if (user_clicked)
            ImGui::SetKeyOwner(ImGuiKey_MouseLeft, id);
        g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
        if (is_multiline || (flags & ImGuiInputTextFlags_CallbackHistory))
        {
            g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Up) | (1 << ImGuiDir_Down);
            ImGui::SetKeyOwner(ImGuiKey_UpArrow, id);
            ImGui::SetKeyOwner(ImGuiKey_DownArrow, id);
        }
        if (is_multiline)
        {
            ImGui::SetKeyOwner(ImGuiKey_PageUp, id);
            ImGui::SetKeyOwner(ImGuiKey_PageDown, id);
        }
        // FIXME: May be a problem to always steal Alt on OSX, would ideally still allow an uninterrupted Alt down-up to toggle menu
        if (is_osx)
            ImGui::SetKeyOwner(ImGuiMod_Alt, id);

        // Expose scroll in a manner that is agnostic to us using a child window
        if (is_multiline && state != NULL)
            state->Scroll.y = draw_window->Scroll.y;
    }

    // We have an edge case if ActiveId was set through another widget (e.g. widget being swapped), clear id immediately (don't wait until the end of the function)
    if (g.ActiveId == id && state == NULL)
        ImGui::ClearActiveID();

    // Release focus when we click outside
    if (g.ActiveId == id && io.MouseClicked[0] && !init_state && !init_make_active) //-V560
        clear_active_id = true;

    // Lock the decision of whether we are going to take the path displaying the cursor or selection
    bool render_cursor = (g.ActiveId == id) || (state && user_scroll_active);
    bool render_selection = state && (state->HasSelection() || select_all) && (RENDER_SELECTION_WHEN_INACTIVE || render_cursor);
    bool value_changed = false;
    bool validated = false;

    // Select the buffer to render.
    const bool buf_display_from_state = (render_cursor || render_selection || g.ActiveId == id) && !is_readonly && state;
    const bool is_displaying_hint = (label.ch() != NULL && (buf_display_from_state ? state->TextA.Data : buf)[0] == 0);

    // Password pushes a temporary font with only a fallback glyph
    if (is_password && !is_displaying_hint)
    {
        const ImFontGlyph* glyph = g.Font->FindGlyph('*');
        ImFont* password_font = &g.InputTextPasswordFont;
        password_font->FontSize = g.Font->FontSize;
        password_font->Scale = g.Font->Scale;
        password_font->Ascent = g.Font->Ascent;
        password_font->Descent = g.Font->Descent;
        password_font->ContainerAtlas = g.Font->ContainerAtlas;
        password_font->FallbackGlyph = glyph;
        password_font->FallbackAdvanceX = glyph->AdvanceX;
        IM_ASSERT(password_font->Glyphs.empty() && password_font->IndexAdvanceX.empty() && password_font->IndexLookup.empty());
        ImGui::PushFont(password_font);
    }

    bool is_mouse_moved = false;

    // Process mouse inputs and character inputs
    if (g.ActiveId == id)
    {
        IM_ASSERT(state != NULL);
        state->Edited = false;
        state->BufCapacity = buf_size;
        state->Flags = flags;

        // Although we are active we don't prevent mouse from hovering other elements unless we are interacting right now with the widget.
        // Down the line we should have a cleaner library-wide concept of Selected vs Active.
        g.ActiveIdAllowOverlap = !io.MouseDown[0];

        // Edit in progress
        const float mouse_x = (io.MousePos.x - bb.Min.x - style.FramePadding.x) + state->Scroll.x + style.FramePadding.x;
        const float mouse_y = (is_multiline ? (io.MousePos.y - draw_window->DC.CursorPos.y) : (g.FontSize * 0.5f));

        if (select_all)
        {
            state->SelectAll();
            state->SelectedAllMouseLock = true;
        }
        else if (hovered && io.MouseClickedCount[0] >= 2 && !io.KeyShift)
        {
            stb_textedit_click(state, state->Stb, mouse_x, mouse_y);
            const int multiclick_count = (io.MouseClickedCount[0] - 2);
            if ((multiclick_count % 2) == 0)
            {
                // Double-click: Select word
                // We always use the "Mac" word advance for double-click select vs CTRL+Right which use the platform dependent variant:
                // FIXME: There are likely many ways to improve this behavior, but there's no "right" behavior (depends on use-case, software, OS)
                const bool is_bol = (state->Stb->cursor == 0) || ImStb::STB_TEXTEDIT_GETCHAR(state, state->Stb->cursor - 1) == '\n';
                if (STB_TEXT_HAS_SELECTION(state->Stb) || !is_bol)
                    state->OnKeyPressed(STB_TEXTEDIT_K_WORDLEFT);
                //state->OnKeyPressed(STB_TEXTEDIT_K_WORDRIGHT | STB_TEXTEDIT_K_SHIFT);
                if (!STB_TEXT_HAS_SELECTION(state->Stb))
                    ImStb::stb_textedit_prep_selection_at_cursor(state->Stb);
                state->Stb->cursor = ImStb::STB_TEXTEDIT_MOVEWORDRIGHT_MAC(state, state->Stb->cursor);
                state->Stb->select_end = state->Stb->cursor;
                ImStb::stb_textedit_clamp(state, state->Stb);
            }
            else
            {
                // Triple-click: Select line
                const bool is_eol = ImStb::STB_TEXTEDIT_GETCHAR(state, state->Stb->cursor) == '\n';
                state->OnKeyPressed(STB_TEXTEDIT_K_LINESTART);
                state->OnKeyPressed(STB_TEXTEDIT_K_LINEEND | STB_TEXTEDIT_K_SHIFT);
                state->OnKeyPressed(STB_TEXTEDIT_K_RIGHT | STB_TEXTEDIT_K_SHIFT);
                if (!is_eol && is_multiline)
                {
                    ImSwap(state->Stb->select_start, state->Stb->select_end);
                    state->Stb->cursor = state->Stb->select_end;
                }
                state->CursorFollow = false;
            }
            state->CursorAnimReset();
        }
        else if (io.MouseClicked[0] && !state->SelectedAllMouseLock)
        {
            if (hovered)
            {
                if (io.KeyShift)
                    stb_textedit_drag(state, state->Stb, mouse_x, mouse_y);
                else
                    stb_textedit_click(state, state->Stb, mouse_x, mouse_y);
                state->CursorAnimReset();
            }
        }
        else if (io.MouseDown[0] && !state->SelectedAllMouseLock && (io.MouseDelta.x != 0.0f || io.MouseDelta.y != 0.0f))
        {
            stb_textedit_drag(state, state->Stb, mouse_x, mouse_y);
            state->CursorAnimReset();
            state->CursorFollow = true;
        }
        if (state->SelectedAllMouseLock && !io.MouseDown[0])
            state->SelectedAllMouseLock = false;

        // We expect backends to emit a Tab key but some also emit a Tab character which we ignore (#2467, #1336)
        // (For Tab and Enter: Win32/SFML/Allegro are sending both keys and chars, GLFW and SDL are only sending keys. For Space they all send all threes)
        if ((flags & ImGuiInputTextFlags_AllowTabInput) && !is_readonly)
        {
            if (ImGui::Shortcut(ImGuiKey_Tab, ImGuiInputFlags_Repeat, id))
            {
                unsigned int c = '\t'; // Insert TAB
                if (InputTextFilterCharacter(&g, &c, flags, callback, callback_user_data))
                    state->OnCharPressed(c);
            }
            // FIXME: Implement Shift+Tab
            /*
            if (Shortcut(ImGuiKey_Tab | ImGuiMod_Shift, ImGuiInputFlags_Repeat, id))
            {
            }
            */
        }

        // Process regular text input (before we check for Return because using some IME will effectively send a Return?)
        // We ignore CTRL inputs, but need to allow ALT+CTRL as some keyboards (e.g. German) use AltGR (which _is_ Alt+Ctrl) to input certain characters.
        const bool ignore_char_inputs = (io.KeyCtrl && !io.KeyAlt) || (is_osx && io.KeyCtrl);
        if (io.InputQueueCharacters.Size > 0)
        {
            if (!ignore_char_inputs && !is_readonly && !input_requested_by_nav)
                for (int n = 0; n < io.InputQueueCharacters.Size; n++)
                {
                    // Insert character if they pass filtering
                    unsigned int c = (unsigned int)io.InputQueueCharacters[n];
                    if (c == '\t') // Skip Tab, see above.
                        continue;
                    if (InputTextFilterCharacter(&g, &c, flags, callback, callback_user_data))
                        state->OnCharPressed(c);
                }

            // Consume characters
            io.InputQueueCharacters.resize(0);
        }
    }

    // Process other shortcuts/key-presses
    bool revert_edit = false;
    if (g.ActiveId == id && !g.ActiveIdIsJustActivated && !clear_active_id)
    {
        IM_ASSERT(state != NULL);

        const int row_count_per_page = ImMax((int)((inner_size.y - style.FramePadding.y) / g.FontSize), 1);
        state->Stb->row_count_per_page = row_count_per_page;

        const int k_mask = (io.KeyShift ? STB_TEXTEDIT_K_SHIFT : 0);
        const bool is_wordmove_key_down = is_osx ? io.KeyAlt : io.KeyCtrl;                     // OS X style: Text editing cursor movement using Alt instead of Ctrl
        const bool is_startend_key_down = is_osx && io.KeyCtrl && !io.KeySuper && !io.KeyAlt;  // OS X style: Line/Text Start and End using Cmd+Arrows instead of Home/End

        // Using Shortcut() with ImGuiInputFlags_RouteFocused (default policy) to allow routing operations for other code (e.g. calling window trying to use CTRL+A and CTRL+B: formet would be handled by InputText)
        // Otherwise we could simply assume that we own the keys as we are active.
        const ImGuiInputFlags f_repeat = ImGuiInputFlags_Repeat;
        const bool is_cut = (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_X, f_repeat, id) || ImGui::Shortcut(ImGuiMod_Shift | ImGuiKey_Delete, f_repeat, id)) && !is_readonly && !is_password && (!is_multiline || state->HasSelection());
        const bool is_copy = (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_C, 0, id) || ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Insert, 0, id)) && !is_password && (!is_multiline || state->HasSelection());
        const bool is_paste = (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_V, f_repeat, id) || ImGui::Shortcut(ImGuiMod_Shift | ImGuiKey_Insert, f_repeat, id)) && !is_readonly;
        const bool is_undo = (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Z, f_repeat, id)) && !is_readonly && is_undoable;
        const bool is_redo = (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Y, f_repeat, id) || (is_osx && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Z, f_repeat, id))) && !is_readonly && is_undoable;
        const bool is_select_all = ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_A, 0, id);

        // We allow validate/cancel with Nav source (gamepad) to makes it easier to undo an accidental NavInput press with no keyboard wired, but otherwise it isn't very useful.
        const bool nav_gamepad_active = (io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) != 0 && (io.BackendFlags & ImGuiBackendFlags_HasGamepad) != 0;
        const bool is_enter_pressed = ImGui::IsKeyPressed(ImGuiKey_Enter, true) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter, true);
        const bool is_gamepad_validate = nav_gamepad_active && (ImGui::IsKeyPressed(ImGuiKey_NavGamepadActivate, false) || ImGui::IsKeyPressed(ImGuiKey_NavGamepadInput, false));
        const bool is_cancel = ImGui::Shortcut(ImGuiKey_Escape, f_repeat, id) || (nav_gamepad_active && ImGui::Shortcut(ImGuiKey_NavGamepadCancel, f_repeat, id));

        // FIXME: Should use more Shortcut() and reduce IsKeyPressed()+SetKeyOwner(), but requires modifiers combination to be taken account of.
        // FIXME-OSX: Missing support for Alt(option)+Right/Left = go to end of line, or next line if already in end of line.
        if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) { state->OnKeyPressed((is_startend_key_down ? STB_TEXTEDIT_K_LINESTART : is_wordmove_key_down ? STB_TEXTEDIT_K_WORDLEFT : STB_TEXTEDIT_K_LEFT) | k_mask); }
        else if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) { state->OnKeyPressed((is_startend_key_down ? STB_TEXTEDIT_K_LINEEND : is_wordmove_key_down ? STB_TEXTEDIT_K_WORDRIGHT : STB_TEXTEDIT_K_RIGHT) | k_mask); }
        else if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) && is_multiline) { if (io.KeyCtrl) ImGui::SetScrollY(draw_window, ImMax(draw_window->Scroll.y - g.FontSize, 0.0f)); else state->OnKeyPressed((is_startend_key_down ? STB_TEXTEDIT_K_TEXTSTART : STB_TEXTEDIT_K_UP) | k_mask); }
        else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) && is_multiline) { if (io.KeyCtrl) ImGui::SetScrollY(draw_window, ImMin(draw_window->Scroll.y + g.FontSize, ImGui::GetScrollMaxY())); else state->OnKeyPressed((is_startend_key_down ? STB_TEXTEDIT_K_TEXTEND : STB_TEXTEDIT_K_DOWN) | k_mask); }
        else if (ImGui::IsKeyPressed(ImGuiKey_PageUp) && is_multiline) { state->OnKeyPressed(STB_TEXTEDIT_K_PGUP | k_mask); scroll_y -= row_count_per_page * g.FontSize; }
        else if (ImGui::IsKeyPressed(ImGuiKey_PageDown) && is_multiline) { state->OnKeyPressed(STB_TEXTEDIT_K_PGDOWN | k_mask); scroll_y += row_count_per_page * g.FontSize; }
        else if (ImGui::IsKeyPressed(ImGuiKey_Home)) { state->OnKeyPressed(io.KeyCtrl ? STB_TEXTEDIT_K_TEXTSTART | k_mask : STB_TEXTEDIT_K_LINESTART | k_mask); }
        else if (ImGui::IsKeyPressed(ImGuiKey_End)) { state->OnKeyPressed(io.KeyCtrl ? STB_TEXTEDIT_K_TEXTEND | k_mask : STB_TEXTEDIT_K_LINEEND | k_mask); }
        else if (ImGui::IsKeyPressed(ImGuiKey_Delete) && !is_readonly && !is_cut)
        {
            if (!state->HasSelection())
            {
                // OSX doesn't seem to have Super+Delete to delete until end-of-line, so we don't emulate that (as opposed to Super+Backspace)
                if (is_wordmove_key_down)
                    state->OnKeyPressed(STB_TEXTEDIT_K_WORDRIGHT | STB_TEXTEDIT_K_SHIFT);
            }
            state->OnKeyPressed(STB_TEXTEDIT_K_DELETE | k_mask);
        }
        else if (ImGui::IsKeyPressed(ImGuiKey_Backspace) && !is_readonly)
        {
            if (!state->HasSelection())
            {
                if (is_wordmove_key_down)
                    state->OnKeyPressed(STB_TEXTEDIT_K_WORDLEFT | STB_TEXTEDIT_K_SHIFT);
                else if (is_osx && io.KeyCtrl && !io.KeyAlt && !io.KeySuper)
                    state->OnKeyPressed(STB_TEXTEDIT_K_LINESTART | STB_TEXTEDIT_K_SHIFT);
            }
            state->OnKeyPressed(STB_TEXTEDIT_K_BACKSPACE | k_mask);
        }
        else if (is_enter_pressed || is_gamepad_validate)
        {
            // Determine if we turn Enter into a \n character
            bool ctrl_enter_for_new_line = (flags & ImGuiInputTextFlags_CtrlEnterForNewLine) != 0;
            if (!is_multiline || is_gamepad_validate || (ctrl_enter_for_new_line && !io.KeyCtrl) || (!ctrl_enter_for_new_line && io.KeyCtrl))
            {
                validated = true;
                if (io.ConfigInputTextEnterKeepActive && !is_multiline)
                    state->SelectAll(); // No need to scroll
                else
                    clear_active_id = true;
            }
            else if (!is_readonly)
            {
                unsigned int c = '\n'; // Insert new line
                if (InputTextFilterCharacter(&g, &c, flags, callback, callback_user_data))
                    state->OnCharPressed(c);
            }
        }
        else if (is_cancel)
        {
            if (flags & ImGuiInputTextFlags_EscapeClearsAll)
            {
                if (buf[0] != 0)
                {
                    revert_edit = true;
                }
                else
                {
                    render_cursor = render_selection = false;
                    clear_active_id = true;
                }
            }
            else
            {
                clear_active_id = revert_edit = true;
                render_cursor = render_selection = false;
            }
        }
        else if (is_undo || is_redo)
        {
            state->OnKeyPressed(is_undo ? STB_TEXTEDIT_K_UNDO : STB_TEXTEDIT_K_REDO);
            state->ClearSelection();
        }
        else if (is_select_all)
        {
            state->SelectAll();
            state->CursorFollow = true;
        }
        else if (is_cut || is_copy)
        {
            // Cut, Copy
            if (g.PlatformIO.Platform_SetClipboardTextFn != NULL)
            {
                const int ib = state->HasSelection() ? ImMin(state->Stb->select_start, state->Stb->select_end) : 0;
                const int ie = state->HasSelection() ? ImMax(state->Stb->select_start, state->Stb->select_end) : state->TextLen;

                char backup = state->TextA.Data[ie];
                state->TextA.Data[ie] = 0; // A bit of a hack since SetClipboardText only takes null terminated strings
                ImGui::SetClipboardText(state->TextA.Data + ib);
                state->TextA.Data[ie] = backup;
            }
            if (is_cut)
            {
                if (!state->HasSelection())
                    state->SelectAll();
                state->CursorFollow = true;
                stb_textedit_cut(state, state->Stb);
            }
        }
        else if (is_paste)
        {
            if (const char* clipboard = ImGui::GetClipboardText())
            {
                // Filter pasted buffer
                const int clipboard_len = (int)strlen(clipboard);
                char* clipboard_filtered = (char*)IM_ALLOC(clipboard_len + 1);
                int clipboard_filtered_len = 0;
                for (const char* s = clipboard; *s != 0; )
                {
                    unsigned int c;
                    int len = ImTextCharFromUtf8(&c, s, NULL);
                    s += len;
                    if (!InputTextFilterCharacter(&g, &c, flags, callback, callback_user_data, true))
                        continue;
                    memcpy(clipboard_filtered + clipboard_filtered_len, s - len, len);
                    clipboard_filtered_len += len;
                }
                clipboard_filtered[clipboard_filtered_len] = 0;
                if (clipboard_filtered_len > 0) // If everything was filtered, ignore the pasting operation
                {
                    stb_textedit_paste(state, state->Stb, clipboard_filtered, clipboard_filtered_len);
                    state->CursorFollow = true;
                }
                ImGui::MemFree(clipboard_filtered);
            }
        }

        // Update render selection flag after events have been handled, so selection highlight can be displayed during the same frame.
        render_selection |= state->HasSelection() && (RENDER_SELECTION_WHEN_INACTIVE || render_cursor);
    }

    // Process callbacks and apply result back to user's buffer.
    const char* apply_new_text = NULL;
    int apply_new_text_length = 0;
    if (g.ActiveId == id)
    {
        IM_ASSERT(state != NULL);
        if (revert_edit && !is_readonly)
        {
            if (flags & ImGuiInputTextFlags_EscapeClearsAll)
            {
                // Clear input
                IM_ASSERT(buf[0] != 0);
                apply_new_text = "";
                apply_new_text_length = 0;
                value_changed = true;
                IMSTB_TEXTEDIT_CHARTYPE empty_string;
                stb_textedit_replace(state, state->Stb, &empty_string, 0);
            }
            else if (strcmp(buf, state->TextToRevertTo.Data) != 0)
            {
                apply_new_text = state->TextToRevertTo.Data;
                apply_new_text_length = state->TextToRevertTo.Size - 1;

                // Restore initial value. Only return true if restoring to the initial value changes the current buffer contents.
                // Push records into the undo stack so we can CTRL+Z the revert operation itself
                value_changed = true;
                stb_textedit_replace(state, state->Stb, state->TextToRevertTo.Data, state->TextToRevertTo.Size - 1);
            }
        }

        // When using 'ImGuiInputTextFlags_EnterReturnsTrue' as a special case we reapply the live buffer back to the input buffer
        // before clearing ActiveId, even though strictly speaking it wasn't modified on this frame.
        // If we didn't do that, code like InputInt() with ImGuiInputTextFlags_EnterReturnsTrue would fail.
        // This also allows the user to use InputText() with ImGuiInputTextFlags_EnterReturnsTrue without maintaining any user-side storage
        // (please note that if you use this property along ImGuiInputTextFlags_CallbackResize you can end up with your temporary string object
        // unnecessarily allocating once a frame, either store your string data, either if you don't then don't use ImGuiInputTextFlags_CallbackResize).
        const bool apply_edit_back_to_user_buffer = !revert_edit || (validated && (flags & ImGuiInputTextFlags_EnterReturnsTrue) != 0);
        if (apply_edit_back_to_user_buffer)
        {
            // Apply new value immediately - copy modified buffer back
            // Note that as soon as the input box is active, the in-widget value gets priority over any underlying modification of the input buffer
            // FIXME: We actually always render 'buf' when calling DrawList->AddText, making the comment above incorrect.
            // FIXME-OPT: CPU waste to do this every time the widget is active, should mark dirty state from the stb_textedit callbacks.

            // User callback
            if ((flags & (ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_CallbackEdit | ImGuiInputTextFlags_CallbackAlways)) != 0)
            {
                IM_ASSERT(callback != NULL);

                // The reason we specify the usage semantic (Completion/History) is that Completion needs to disable keyboard TABBING at the moment.
                ImGuiInputTextFlags event_flag = 0;
                ImGuiKey event_key = ImGuiKey_None;
                if ((flags & ImGuiInputTextFlags_CallbackCompletion) != 0 && ImGui::Shortcut(ImGuiKey_Tab, 0, id))
                {
                    event_flag = ImGuiInputTextFlags_CallbackCompletion;
                    event_key = ImGuiKey_Tab;
                }
                else if ((flags & ImGuiInputTextFlags_CallbackHistory) != 0 && ImGui::IsKeyPressed(ImGuiKey_UpArrow))
                {
                    event_flag = ImGuiInputTextFlags_CallbackHistory;
                    event_key = ImGuiKey_UpArrow;
                }
                else if ((flags & ImGuiInputTextFlags_CallbackHistory) != 0 && ImGui::IsKeyPressed(ImGuiKey_DownArrow))
                {
                    event_flag = ImGuiInputTextFlags_CallbackHistory;
                    event_key = ImGuiKey_DownArrow;
                }
                else if ((flags & ImGuiInputTextFlags_CallbackEdit) && state->Edited)
                {
                    event_flag = ImGuiInputTextFlags_CallbackEdit;
                }
                else if (flags & ImGuiInputTextFlags_CallbackAlways)
                {
                    event_flag = ImGuiInputTextFlags_CallbackAlways;
                }

                if (event_flag)
                {
                    ImGuiInputTextCallbackData callback_data;
                    callback_data.Ctx = &g;
                    callback_data.EventFlag = event_flag;
                    callback_data.Flags = flags;
                    callback_data.UserData = callback_user_data;

                    // FIXME-OPT: Undo stack reconcile needs a backup of the data until we rework API, see #7925
                    state->CallbackTextBackup.resize(state->TextLen + 1);
                    memcpy(state->CallbackTextBackup.Data, state->TextA.Data, state->TextLen + 1);

                    char* callback_buf = is_readonly ? buf : state->TextA.Data;
                    callback_data.EventKey = event_key;
                    callback_data.Buf = callback_buf;
                    callback_data.BufTextLen = state->TextLen;
                    callback_data.BufSize = state->BufCapacity;
                    callback_data.BufDirty = false;

                    const int utf8_cursor_pos = callback_data.CursorPos = state->Stb->cursor;
                    const int utf8_selection_start = callback_data.SelectionStart = state->Stb->select_start;
                    const int utf8_selection_end = callback_data.SelectionEnd = state->Stb->select_end;

                    // Call user code
                    callback(&callback_data);

                    // Read back what user may have modified
                    callback_buf = is_readonly ? buf : state->TextA.Data; // Pointer may have been invalidated by a resize callback
                    IM_ASSERT(callback_data.Buf == callback_buf);         // Invalid to modify those fields
                    IM_ASSERT(callback_data.BufSize == state->BufCapacity);
                    IM_ASSERT(callback_data.Flags == flags);
                    const bool buf_dirty = callback_data.BufDirty;
                    if (callback_data.CursorPos != utf8_cursor_pos || buf_dirty) { state->Stb->cursor = callback_data.CursorPos; state->CursorFollow = true; }
                    if (callback_data.SelectionStart != utf8_selection_start || buf_dirty) { state->Stb->select_start = (callback_data.SelectionStart == callback_data.CursorPos) ? state->Stb->cursor : callback_data.SelectionStart; }
                    if (callback_data.SelectionEnd != utf8_selection_end || buf_dirty) { state->Stb->select_end = (callback_data.SelectionEnd == callback_data.SelectionStart) ? state->Stb->select_start : callback_data.SelectionEnd; }
                    if (buf_dirty)
                    {
                        // Callback may update buffer and thus set buf_dirty even in read-only mode.
                        IM_ASSERT(callback_data.BufTextLen == (int)strlen(callback_data.Buf)); // You need to maintain BufTextLen if you change the text!
                        InputTextReconcileUndoStateAfterUserCallback(state, callback_data.Buf, callback_data.BufTextLen); // FIXME: Move the rest of this block inside function and rename to InputTextReconcileStateAfterUserCallback() ?
                        state->TextLen = callback_data.BufTextLen;  // Assume correct length and valid UTF-8 from user, saves us an extra strlen()
                        state->CursorAnimReset();
                    }
                }
            }

            // Will copy result string if modified
            if (!is_readonly && strcmp(state->TextA.Data, buf) != 0)
            {
                apply_new_text = state->TextA.Data;
                apply_new_text_length = state->TextLen;
                value_changed = true;
            }
        }
    }

    // Handle reapplying final data on deactivation (see InputTextDeactivateHook() for details)
    if (g.InputTextDeactivatedState.ID == id)
    {
        if (g.ActiveId != id && ImGui::IsItemDeactivatedAfterEdit() && !is_readonly && strcmp(g.InputTextDeactivatedState.TextA.Data, buf) != 0)
        {
            apply_new_text = g.InputTextDeactivatedState.TextA.Data;
            apply_new_text_length = g.InputTextDeactivatedState.TextA.Size - 1;
            value_changed = true;
            //IMGUI_DEBUG_LOG("InputText(): apply Deactivated data for 0x%08X: \"%.*s\".\n", id, apply_new_text_length, apply_new_text);
        }
        g.InputTextDeactivatedState.ID = 0;
    }

    // Copy result to user buffer. This can currently only happen when (g.ActiveId == id)
    if (apply_new_text != NULL)
    {
        //// We cannot test for 'backup_current_text_length != apply_new_text_length' here because we have no guarantee that the size
        //// of our owned buffer matches the size of the string object held by the user, and by design we allow InputText() to be used
        //// without any storage on user's side.
        IM_ASSERT(apply_new_text_length >= 0);
        if (is_resizable)
        {
            ImGuiInputTextCallbackData callback_data;
            callback_data.Ctx = &g;
            callback_data.EventFlag = ImGuiInputTextFlags_CallbackResize;
            callback_data.Flags = flags;
            callback_data.Buf = buf;
            callback_data.BufTextLen = apply_new_text_length;
            callback_data.BufSize = ImMax(buf_size, apply_new_text_length + 1);
            callback_data.UserData = callback_user_data;
            callback(&callback_data);
            buf = callback_data.Buf;
            buf_size = callback_data.BufSize;
            apply_new_text_length = ImMin(callback_data.BufTextLen, buf_size - 1);
            IM_ASSERT(apply_new_text_length <= buf_size);
        }
        //IMGUI_DEBUG_PRINT("InputText(\"%s\"): apply_new_text length %d\n", label, apply_new_text_length);

        // If the underlying buffer resize was denied or not carried to the next frame, apply_new_text_length+1 may be >= buf_size.
        ImStrncpy(buf, apply_new_text, ImMin(apply_new_text_length + 1, buf_size));
    }

    // Release active ID at the end of the function (so e.g. pressing Return still does a final application of the value)
    // Otherwise request text input ahead for next frame.
    if (g.ActiveId == id && clear_active_id)
        ImGui::ClearActiveID();
    else if (g.ActiveId == id)
        g.WantTextInputNextFrame = 1;

    // Animate border
    std::string border_anim_name = "InputText/Border/Alpha#"s + std::to_string(label.id);
    std::string border_grow_anim_name = "InputText/Border/Grow#"s + std::to_string(label.id);

    bool anim_condition = hovered || render_cursor;
    bool anim_condition2 = render_cursor;
    auto border_builder = AnimBuilder<float>().range(0.f, 1.f).condition(anim_condition2).duration(milliseconds(200)).easing(Easing::QuadraticEaseIn).play_policy(PlayPolicy::ENDLESS);
    float border = border_builder.build(border_anim_name, ImGui::GetIO().DeltaTime);

    auto border_grow_builder = AnimBuilder<float>().range(0.f, 1.f).condition(anim_condition2).duration(milliseconds(250)).easing(Easing::BackEaseOut).play_policy(PlayPolicy::ENDLESS);
    float border_grow = border_grow_builder.build(border_grow_anim_name, ImGui::GetIO().DeltaTime);

    // Render frame
    ImGui::RenderFrame(total_bb.Min, total_bb.Max, u32color(COLORS::FOREGROUND), false, style.FrameRounding);

    ImRect border_rect(total_bb.Min - ImVec2(10, 5) * (1.f - border_grow), total_bb.Max + ImVec2(10, 5) * (1.f - border_grow));
    draw_window->DrawList->AddRect(border_rect.Min, border_rect.Max, u32color(COLORS::MAIN, border), style.FrameRounding, 0, 1.1f);

    const ImVec4 clip_rect(bb.Min.x, bb.Min.y, bb.Max.x, bb.Max.y); // Not using frame_bb.Max because we have adjusted size

    ImVec2 draw_pos = bb.Min + ImVec2(0, (bb.GetHeight() - label_size.y) * 0.5f);
    ImVec2 text_size(0.0f, 0.0f);

    // Set upper limit of single-line InputTextEx() at 2 million characters strings. The current pathological worst case is a long line
    // without any carriage return, which would makes ImFont::RenderText() reserve too many vertices and probably crash. Avoid it altogether.
    // Note that we only use this limit on single-line InputText(), so a pathologically large line on a InputTextMultiline() would still crash.
    const int buf_display_max_length = 2 * 1024 * 1024;
    const char* buf_display = buf_display_from_state ? state->TextA.Data : buf; //-V595
    const char* buf_display_end = NULL; // We have specialized paths below for setting the length
    if (!render_cursor && is_displaying_hint)
    {
        buf_display = label.ch();
        buf_display_end = label.ch() + strlen(label.ch());
    }

    // Animate text color
    const std::string col_anim_name = "InputText/TextColor#"s + std::to_string(label.id);
    const ImVec4 default_col = color(COLORS::SECONDARY);
    const ImVec4 actived_col = color(COLORS::WHITE);

    auto col_builder = AnimBuilder<ImVec4>().range(default_col, actived_col).duration(milliseconds(150)).easing(Easing::QuadraticEaseOut).condition(anim_condition).play_policy(PlayPolicy::ENDLESS);
    ImVec4 col = col_builder.build(col_anim_name, ImGui::GetIO().DeltaTime);

    const std::string anim_name = "InputTextSmall/Cursor#"s + std::to_string(label.id);
    auto& active_anims = get_active_anims<float>();

    static std::map<std::string, bool> cursors_moved_data;
    static std::map<std::string, ImVec4> cursors_y_data;
    // Render text. We currently only render selection when the widget is active or while scrolling.
    // FIXME: We could remove the '&& render_cursor' to keep rendering selection when inactive.
    if (render_cursor || render_selection)
    {
        IM_ASSERT(state != NULL);
        if (!is_displaying_hint)
            buf_display_end = buf_display + state->TextLen;

        // Render text (with cursor and selection)
        // This is going to be messy. We need to:
        // - Display the text (this alone can be more easily clipped)
        // - Handle scrolling, highlight selection, display cursor (those all requires some form of 1d->2d cursor position calculation)
        // - Measure text height (for scrollbar)
        // We are attempting to do most of that in **one main pass** to minimize the computation cost (non-negligible for large amount of text) + 2nd pass for selection rendering (we could merge them by an extra refactoring effort)
        // FIXME: This should occur on buf_display but we'd need to maintain cursor/select_start/select_end for UTF-8.
        const char* text_begin = state->TextA.Data;
        const char* text_end = text_begin + state->TextLen;
        ImVec2 cursor_offset, select_start_offset;
        {
            // Find lines numbers straddling cursor and selection min position
            int cursor_line_no = render_cursor ? -1 : -1000;
            int selmin_line_no = render_selection ? -1 : -1000;
            const char* cursor_ptr = render_cursor ? text_begin + state->Stb->cursor : NULL;
            const char* selmin_ptr = render_selection ? text_begin + ImMin(state->Stb->select_start, state->Stb->select_end) : NULL;

            // Count lines and find line number for cursor and selection ends
            int line_count = 1;
            if (is_multiline)
            {
                for (const char* s = text_begin; (s = (const char*)memchr(s, '\n', (size_t)(text_end - s))) != NULL; s++)
                {
                    if (cursor_line_no == -1 && s >= cursor_ptr) { cursor_line_no = line_count; }
                    if (selmin_line_no == -1 && s >= selmin_ptr) { selmin_line_no = line_count; }
                    line_count++;
                }
            }
            if (cursor_line_no == -1) {
                cursor_line_no = line_count;
            }
            if (selmin_line_no == -1)
                selmin_line_no = line_count;

            // Calculate 2d position by finding the beginning of the line and measuring distance
            cursor_offset.x = InputTextCalcTextSize(&g, ImStrbol(cursor_ptr, text_begin), cursor_ptr).x;
            if (cursor_offset.x != InputTextCalcTextSize(&g, ImStrbol(text_end, text_begin), text_end).x)
                is_mouse_moved = true;
            cursor_offset.y = cursor_line_no * g.FontSize;
            if (selmin_line_no >= 0)
            {
                select_start_offset.x = InputTextCalcTextSize(&g, ImStrbol(selmin_ptr, text_begin), selmin_ptr).x;
                select_start_offset.y = selmin_line_no * g.FontSize;
            }



            // Store text height (note that we haven't calculated text width at all, see GitHub issues #383, #1224)
            if (is_multiline)
                text_size = ImVec2(inner_size.x, line_count * g.FontSize);
        }

        // Scroll
        if (render_cursor && state->CursorFollow)
        {
            // Horizontal scroll in chunks of quarter width
            if (!(flags & ImGuiInputTextFlags_NoHorizontalScroll))
            {
                const float scroll_increment_x = inner_size.x * 0.25f;
                const float visible_width = bb.GetWidth();
                if (cursor_offset.x < state->Scroll.x)
                    state->Scroll.x = IM_TRUNC(ImMax(0.0f, cursor_offset.x - scroll_increment_x));
                else if (cursor_offset.x - visible_width >= state->Scroll.x)
                    state->Scroll.x = IM_TRUNC(cursor_offset.x - visible_width + scroll_increment_x);
            }
            else
            {
                state->Scroll.y = 0.0f;
            }

            // Vertical scroll
            if (is_multiline)
            {
                // Test if cursor is vertically visible
                if (cursor_offset.y - g.FontSize < scroll_y)
                    scroll_y = ImMax(0.0f, cursor_offset.y - g.FontSize);
                else if (cursor_offset.y - (inner_size.y - style.FramePadding.y * 2.0f) >= scroll_y)
                    scroll_y = cursor_offset.y - inner_size.y + style.FramePadding.y * 2.0f;
                const float scroll_max_y = ImMax((text_size.y + style.FramePadding.y * 2.0f) - inner_size.y, 0.0f);
                scroll_y = ImClamp(scroll_y, 0.0f, scroll_max_y);
                draw_pos.y += (draw_window->Scroll.y - scroll_y);   // Manipulate cursor pos immediately avoid a frame of lag
                draw_window->Scroll.y = scroll_y;
            }

            state->CursorFollow = false;
        }

        // Draw selection

        const ImVec2 draw_scroll = ImVec2(state->Scroll.x, 0.0f);
        if (render_selection)
        {
            const char* text_selected_begin = text_begin + ImMin(state->Stb->select_start, state->Stb->select_end);
            const char* text_selected_end = text_begin + ImMax(state->Stb->select_start, state->Stb->select_end);

            ImU32 bg_color = ImGui::GetColorU32(ImGuiCol_TextSelectedBg, render_cursor ? 1.0f : 0.6f); // FIXME: current code flow mandate that render_cursor is always true here, we are leaving the transparent one for tests.
            float bg_offy_up = is_multiline ? 0.0f : -1.0f;    // FIXME: those offsets should be part of the style? they don't play so well with multi-line selection.
            float bg_offy_dn = is_multiline ? 0.0f : 2.0f;
            ImVec2 rect_pos = draw_pos + select_start_offset - draw_scroll;
            for (const char* p = text_selected_begin; p < text_selected_end; )
            {
                if (rect_pos.y > clip_rect.w + g.FontSize)
                    break;
                if (rect_pos.y < clip_rect.y)
                {
                    p = (const char*)memchr((void*)p, '\n', text_selected_end - p);
                    p = p ? p + 1 : text_selected_end;
                }
                else
                {
                    ImVec2 rect_size = InputTextCalcTextSize(&g, p, text_selected_end, &p, NULL, true);
                    if (rect_size.x <= 0.0f) rect_size.x = IM_TRUNC(g.Font->GetCharAdvance((ImWchar)' ') * 0.50f); // So we can see selected empty lines
                    ImRect rect(rect_pos + ImVec2(0.0f, bg_offy_up - g.FontSize), rect_pos + ImVec2(rect_size.x, bg_offy_dn));
                    rect.ClipWith(clip_rect);
                    if (rect.Overlaps(clip_rect))
                        draw_window->DrawList->AddRectFilled(rect.Min, rect.Max, bg_color, style.FrameRounding * 0.5f);
                    rect_pos.x = draw_pos.x - draw_scroll.x;
                }
                rect_pos.y += g.FontSize;
            }
        }

        const float target_val = style.FramePadding.x + clip_rect.x + ImGui::CalcTextSize(buf_display, buf_display_end).x;
        float clip_x = target_val;
        if (auto it = active_anims.find(anim_name); !render_selection && !is_mouse_moved && it != active_anims.end())
        {
            if (!cursors_moved_data[label.ch()])
                clip_x = it->second.value;

        }

        // We test for 'buf_display_max_length' as a way to avoid some pathological cases (e.g. single-line 1 MB string) which would make ImDrawList crash.
        // FIXME-OPT: Multiline could submit a smaller amount of contents to AddText() since we already iterated through it.
        if (is_multiline || (buf_display_end - buf_display) < buf_display_max_length)
        {
            ImGui::PushClipRect(ImVec2(bb.Min.x, clip_rect.y), ImVec2(clip_x, clip_rect.z), false);
            draw_window->DrawList->AddText(g.Font, g.FontSize, draw_pos - draw_scroll, ImGui::GetColorU32(col), buf_display, buf_display_end, 0.0f, is_multiline ? NULL : &clip_rect);
            ImGui::PopClipRect();
        }
        // Draw blinking cursor
        if (render_cursor)
        {
            state->CursorAnim += io.DeltaTime;
            ImVec2 cursor_screen_pos = ImTrunc(draw_pos + cursor_offset - draw_scroll);


            auto slider_builder = AnimBuilder<float>();
            slider_builder.range(draw_pos.x, cursor_screen_pos.x).no_startup().easing(Easing::QuadraticEaseInOut).duration(milliseconds(50)).play_policy(PlayPolicy::ENDLESS).dynamic_range(false);

            if (auto it = active_anims.find(anim_name); it != active_anims.end()) {
                auto& active_anim = it->second;

                active_anim.duration = milliseconds(50);

                if (value_changed && active_anim.to != cursor_screen_pos.x) {
                    active_anim.reset_to(cursor_screen_pos.x);
                }
                if (cursor_screen_pos.x != active_anim.value)
                    active_anim.reset_to(cursor_screen_pos.x);

                if (is_mouse_moved)
                    cursors_moved_data[label.ch()] = true;
                else {
                    if (cursors_moved_data[label.ch()]) {
                        if (active_anim.value >= active_anim.to - 5.f)
                            cursors_moved_data[label.ch()] = false;
                    }
                }
            }

            float move = slider_builder.build(anim_name, ImGui::GetIO().DeltaTime);

            ImRect cursor_screen_rect(move, cursor_screen_pos.y - g.FontSize + 0.5f, move + 1.0f, cursor_screen_pos.y - 1.5f);

            bool cursor_is_visible = (!g.IO.ConfigInputTextCursorBlink) || (state->CursorAnim <= 0.0f) || ImFmod(state->CursorAnim, 1.20f) <= 0.80f;
            if (cursor_is_visible && cursor_screen_rect.Overlaps(clip_rect))
                draw_window->DrawList->AddLine(cursor_screen_rect.Min, cursor_screen_rect.GetBL(), ImGui::GetColorU32(ImGuiCol_Text));
            cursors_y_data[label.ch()] = ImVec4(cursor_screen_rect.Min.y, cursor_screen_rect.GetBL().y, cursor_screen_pos.x, 0.f);

            // Notify OS of text input position for advanced IME (-1 x offset so that Windows IME can cover our cursor. Bit of an extra nicety.)
            if (!is_readonly)
            {
                g.PlatformImeData.WantVisible = true;
                g.PlatformImeData.InputPos = ImVec2(cursor_screen_pos.x - 1.0f, cursor_screen_pos.y - g.FontSize);
                g.PlatformImeData.InputLineHeight = g.FontSize;
            }
        }
    }
    else
    {

        // Render text only (no selection, no cursor)
        if (is_multiline)
            text_size = ImVec2(inner_size.x, InputTextCalcTextLenAndLineCount(buf_display, &buf_display_end) * g.FontSize); // We don't need width
        else if (!is_displaying_hint && g.ActiveId == id)
            buf_display_end = buf_display + state->TextLen;
        else if (!is_displaying_hint)
            buf_display_end = buf_display + strlen(buf_display);

        if (is_multiline || (buf_display_end - buf_display) < buf_display_max_length)
        {
            draw_window->DrawList->AddText(g.Font, g.FontSize, draw_pos, ImGui::GetColorU32(col), buf_display, buf_display_end, 0.0f, is_multiline ? NULL : &clip_rect);
        }

        if (auto it = active_anims.find(anim_name); it != active_anims.end()) {
            auto& cursors_data = cursors_y_data[label.ch()];

            if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
                cursors_data.z = 1.f;

            if (cursors_data.z != 1.f) {
                it->second.duration = milliseconds(250);

                if (!ImGui::IsMouseDragging(ImGuiMouseButton_Left) && it->second.to != draw_pos.x)
                    it->second.reset_to(draw_pos.x);

                auto slider_builder = AnimBuilder<float>();
                slider_builder.range(0, 0).no_startup().easing(Easing::QuadraticEaseInOut).duration(milliseconds(50)).play_policy(PlayPolicy::ENDLESS).dynamic_range(false);

                float move = slider_builder.build(anim_name, ImGui::GetIO().DeltaTime);

                if (!ImGui::IsMouseDragging(ImGuiMouseButton_Left) && move != draw_pos.x) {
                    draw_window->DrawList->AddLine(ImVec2(move, cursors_data.x), ImVec2(move + 1.f, cursors_data.y), ImGui::GetColorU32(col));
                }
            }
        }

    }

    if (is_password && !is_displaying_hint)
        ImGui::PopFont();

    if (icon_width > 0) {
        const ImVec2 icon_size = ImGui::CalcTextSize(icon);
        ImVec2 icon_pos(total_bb.Max.x - style.FramePadding.x - icon_size.x, total_bb.GetCenter().y - icon_size.y * 0.5f);
        draw_window->DrawList->AddText(g.Font, g.FontSize, icon_pos, ImGui::GetColorU32(col), icon);
    }

    // Log as text
    if (g.LogEnabled && (!is_password || is_displaying_hint))
    {
        ImGui::LogSetNextTextDecoration("{", "}");
        ImGui::LogRenderedText(&draw_pos, buf_display, buf_display_end);
    }

    //if (label_size.x > 0) {
    //    const ImVec2 text_pos = bb.Min + ImVec2(0, (bb.GetHeight() - label_size.y) * 0.5f);
    //    ImGui::RenderText(text_pos, label);
    //}

    //draw_window->DrawList->AddText(g.Font, g.FontSize, draw_pos, ImGui::GetColorU32(col), icon, nullptr, 0.0f);

    if (value_changed)
        ImGui::MarkItemEdited(id);

    if (render_cursor)
        g.MouseCursor = ImGuiMouseCursor_TextInput;
    else if (hovered)
        g.MouseCursor = ImGuiMouseCursor_Hand;

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Inputable);
    if ((flags & ImGuiInputTextFlags_EnterReturnsTrue) != 0)
        return validated;
    else
        return value_changed;
}

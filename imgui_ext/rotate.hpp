#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS

#include <cmath>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace ImRotations {
    constexpr float deg2rad(float degrees) {
        return degrees * (M_PI / 180.0f);
    }

    int rotation_start_index;
    void Start() {
        rotation_start_index = ImGui::GetWindowDrawList()->VtxBuffer.Size;
    }

    ImVec2 Center() {
        ImVec2 l(FLT_MAX, FLT_MAX), u(-FLT_MAX, -FLT_MAX);

        const auto& buf = ImGui::GetWindowDrawList()->VtxBuffer;
        for (int i = rotation_start_index; i < buf.Size; i++) {
            l = ImMin(l, buf[i].pos);
            u = ImMax(u, buf[i].pos);
        }

        return ImVec2((l.x + u.x) / 2, (l.y + u.y) / 2); // _ClipRectStack
    }

    void End(float rad, ImVec2 center = Center()) {
        float s = sinf(rad), c = cosf(rad);
        center = ImRotate(center, s, c) - center;

        auto& buf = ImGui::GetWindowDrawList()->VtxBuffer;
        for (int i = rotation_start_index; i < buf.Size; i++) {
            buf[i].pos = ImRotate(buf[i].pos, s, c) - center;
        }
    }
}
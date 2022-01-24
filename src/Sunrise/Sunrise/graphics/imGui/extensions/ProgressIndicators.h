#pragma once

#include "srpch.h"
#include "imgui.h"

namespace ImGui {

    //bool BufferingBar(const char* label, float value, const ImVec2& size_arg, const ImU32& bg_col, const ImU32& fg_col) {
    //    ImGuiWindow* window = GetCurrentWindow();
    //    if (window->SkipItems)
    //        return false;

    //    ImGuiContext& g = *GImGui;
    //    const ImGuiStyle& style = g.Style;
    //    const ImGuiID id = window->GetID(label);

    //    ImVec2 pos = window->DC.CursorPos;
    //    ImVec2 size = size_arg;
    //    size.x -= style.FramePadding.x * 2;

    //    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    //    ItemSize(bb, style.FramePadding.y);
    //    if (!ItemAdd(bb, id))
    //        return false;

    //    // Render
    //    const float circleStart = size.x * 0.7f;
    //    const float circleEnd = size.x;
    //    const float circleWidth = circleEnd - circleStart;

    //    window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart, bb.Max.y), bg_col);
    //    window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart * value, bb.Max.y), fg_col);

    //    const float t = g.Time;
    //    const float r = size.y / 2;
    //    const float speed = 1.5f;

    //    const float a = speed * 0;
    //    const float b = speed * 0.333f;
    //    const float c = speed * 0.666f;

    //    const float o1 = (circleWidth + r) * (t + a - speed * (int)((t + a) / speed)) / speed;
    //    const float o2 = (circleWidth + r) * (t + b - speed * (int)((t + b) / speed)) / speed;
    //    const float o3 = (circleWidth + r) * (t + c - speed * (int)((t + c) / speed)) / speed;

    //    window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o1, bb.Min.y + r), r, bg_col);
    //    window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o2, bb.Min.y + r), r, bg_col);
    //    window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o3, bb.Min.y + r), r, bg_col);
    //}

   /* inline void LoadingIndicator(u32 started_showing_at) {
        float scale = platform_get_pixel_ratio();
        ImVec2 cursor = ImGui::GetCursorScreenPos() + (ImVec2(12, 12) * scale);
        const float speed_scale = 10.0f;
        float cos = cosf(tick / speed_scale);
        float sin = sinf(tick / speed_scale);
        float size = scale * 10.0f;

        u32 alpha = (u32)roundf(lerp(started_showing_at, tick, 255, 14));

        ImGui::GetWindowDrawList()->AddQuadFilled(
            cursor + ImRotate(ImVec2(-size, -size), cos, sin),
            cursor + ImRotate(ImVec2(+size, -size), cos, sin),
            cursor + ImRotate(ImVec2(+size, +size), cos, sin),
            cursor + ImRotate(ImVec2(-size, +size), cos, sin),
            IM_COL32(0, 255, 200, alpha)
        );
    }*/

    //bool Spinner(const char* label, float radius, int thickness, const ImU32& color) {
    //    ImGuiWindow* window = GetCurrentWindow();
    //    if (window->SkipItems)
    //        return false;

    //    ImGuiContext& g = *GImGui;
    //    const ImGuiStyle& style = g.Style;
    //    const ImGuiID id = window->GetID(label);

    //    ImVec2 pos = window->DC.CursorPos;
    //    ImVec2 size((radius) * 2, (radius + style.FramePadding.y) * 2);

    //    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    //    ItemSize(bb, style.FramePadding.y);
    //    if (!ItemAdd(bb, id))
    //        return false;

    //    // Render
    //    window->DrawList->PathClear();

    //    int num_segments = 30;
    //    int start = abs(ImSin(g.Time * 1.8f) * (num_segments - 5));

    //    const float a_min = IM_PI * 2.0f * ((float)start) / (float)num_segments;
    //    const float a_max = IM_PI * 2.0f * ((float)num_segments - 3) / (float)num_segments;

    //    const ImVec2 centre = ImVec2(pos.x + radius, pos.y + radius + style.FramePadding.y);

    //    for (int i = 0; i < num_segments; i++) {
    //        const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
    //        window->DrawList->PathLineTo(ImVec2(centre.x + ImCos(a + g.Time * 8) * radius,
    //            centre.y + ImSin(a + g.Time * 8) * radius));
    //    }

    //    window->DrawList->PathStroke(color, false, thickness);
    //}

}
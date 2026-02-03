#pragma once

#include "glm/ext/vector_float4.hpp"
#include "imgui.h"
#include <string>

namespace Style {
    namespace Color {
        // constexpr glm::vec4 Background = {0.05f, 0.05f, 0.07f, 1.0f};
        // constexpr glm::vec4 Node = {0.25f, 0.65f, 0.95f, 1.0f};
        // constexpr glm::vec4 Outline = {0.10f, 0.10f, 0.12f, 1.0f};
        // constexpr glm::vec4 Line = {0.60f, 0.60f, 0.65f, 1.0f};
        // constexpr glm::vec4 TempLine = {0.50f, 0.50f, 0.55f, 1.0f};

        // constexpr glm::vec4 Background = {0.08f, 0.10f, 0.09f, 1.0f};
        // constexpr glm::vec4 Node = {0.30f, 0.70f, 0.45f, 1.0f};
        // constexpr glm::vec4 Outline = {0.15f, 0.20f, 0.15f, 1.0f};
        // constexpr glm::vec4 Line = {0.60f, 0.75f, 0.60f, 1.0f};
        // constexpr glm::vec4 TempLine = {0.45f, 0.60f, 0.45f, 1.0f};

        constexpr glm::vec4 Background = {0.03f, 0.06f, 0.05f, 1.0f};
        constexpr glm::vec4 Node = {0.85f, 0.95f, 0.90f, 1.0f};
        constexpr glm::vec4 Outline = {0.05f, 0.20f, 0.15f, 1.0f};
        constexpr glm::vec4 Line = {0.00f, 0.80f, 0.55f, 1.0f};
        constexpr glm::vec4 TempLine = {0.15f, 0.50f, 0.45f, 1.0f};

        constexpr glm::vec4 NodeSelected = {0.60f, 0.90f, 0.80f, 1.0f};

    }

    namespace Audio {
        const std::string BasePath = "./assets/audio/";
        const std::string DeleteNode = BasePath + "footstep_snow_003.wav";
        const std::string ReleaseNode = BasePath + "impactMetal_001F.wav";
        const std::string OpenUi = BasePath + "impactGlass_heavy_003.wav";
        const std::string CloseUi = BasePath + "impactPlank_medium_003.wav";
        const std::string SelectNode = BasePath + "impactMetal_002F.wav";
        const std::string ConnectNode = BasePath + "impactGeneric_light_003.wav";
        const std::string DisconnectNode = BasePath + "impactGeneric_light_002.wav";
        const std::string Check = BasePath + "click_003.wav";
        const std::string UnCheck = BasePath + "click_002.wav";
        const std::string OpenTree = BasePath + "click_001.wav";
        const std::string CloseTree = BasePath + "click_005.wav";
    }

    namespace UI {
        constexpr float TextSize = 16.f;
    }

    constexpr int OutlineWidth = 1;
    constexpr int NodeSize = 15;
    constexpr int NodeSelectedPadding = 2;
    constexpr int LineWidth = 10;
    constexpr int LockDragging = 10;
}

static void ApplyNodeEditorStyle()
{
    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowRounding = 12.0f;
    style.FrameRounding = 8.0f;
    style.PopupRounding = 10.0f;
    style.ScrollbarRounding = 10.0f;
    style.GrabRounding = 8.0f;

    style.WindowPadding = ImVec2(16, 14);
    style.FramePadding = ImVec2(4, 4);
    style.ItemSpacing = ImVec2(10, 10);

    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;

    ImVec4* c = style.Colors;

    c[ImGuiCol_WindowBg] = ImVec4(0.04f, 0.04f, 0.04f, 0.97f);
    c[ImGuiCol_ChildBg] = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
    c[ImGuiCol_PopupBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.98f);

    c[ImGuiCol_Border] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);

    c[ImGuiCol_FrameBg] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
    c[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    c[ImGuiCol_FrameBgActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);

    c[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    c[ImGuiCol_TextDisabled]= ImVec4(0.55f, 0.55f, 0.55f, 1.00f);

    c[ImGuiCol_Button] = ImVec4(0.30f, 0.60f, 1.00f, 0.85f);
    c[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.65f, 1.00f, 1.00f);
    c[ImGuiCol_ButtonActive] = ImVec4(0.25f, 0.55f, 0.95f, 1.00f);

    c[ImGuiCol_Header] = ImVec4(0.30f, 0.60f, 1.00f, 0.35f);
    c[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.60f, 1.00f, 0.55f);
    c[ImGuiCol_HeaderActive] = ImVec4(0.30f, 0.60f, 1.00f, 0.75f);

    c[ImGuiCol_CheckMark] = ImVec4(0.30f, 0.60f, 1.00f, 1.00f);
    c[ImGuiCol_SliderGrab] = ImVec4(0.30f, 0.60f, 1.00f, 1.00f);
    c[ImGuiCol_SliderGrabActive] = ImVec4(0.40f, 0.70f, 1.00f, 1.00f);

    c[ImGuiCol_TitleBg] = ImVec4(0.03f, 0.03f, 0.03f, 1.00f);
    c[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
}
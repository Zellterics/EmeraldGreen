#include "ThING/core.h"
#include "ThING/types/apiTypes.h"
#include "ThING/types/enums.h"
#include "ThING/types/renderData.h"
#include "auxiliar/window.h"
#include "glm/fwd.hpp"
#include "globals.h"
#include <ThING/api.h>
#include <cstdint>
#include <imgui.h>
#include <span>
#include <string>
#include <unordered_map>
#include <utility>

#include "auxiliar/style.h"
#include "graph/graph.h"
#include "graph/node.h"
#include "stateMachine/stateMachine.h"

#include "external/Monocraft.h"
#include "external/Monocraft-Bold.h"

void updateCallback(ThING::API& api, FPSCounter& fps){
    static Graph& graph = *editorState.graph; 
    static bool first = true;

    api.getWindowSize(&editorState.windowData.size.width, &editorState.windowData.size.height);

    if(first){
        ImGuiIO& io = ImGui::GetIO();
        ImFontConfig cfg;
        cfg.FontDataOwnedByAtlas = false;
        editorState.MonoFontSize = 18.f;
        editorState.MonoFont = io.Fonts->AddFontFromMemoryTTF(const_cast<uint8_t*>(Monocraft), MonocraftSize, 18.f, &cfg);
        editorState.MonoFontBold = io.Fonts->AddFontFromMemoryTTF(const_cast<uint8_t*>(Monocraft_Bold), Monocraft_BoldSize, 18.f, &cfg);
        ApplyNodeEditorStyle();
        first = false;
    }

    graph.applyLineForces(forces);
    graph.applyNodeRepulsion(forces);
    graph.applyCenterAttraction(forces);
    graph.update();

    ImGuiIO& io = ImGui::GetIO();
    if(!io.WantCaptureMouse){
        stateMachine.update();
    }
}

void uiCallback(ThING::API& api, FPSCounter& fps){
    static Graph& graph = *editorState.graph;
    std::span<InstanceData> circleInstances = api.getInstanceVector(InstanceType::Circle);
    std::span<Node> nodes = graph.viewNodeList();
    for(Node& node : nodes){
        if(!api.exists(node.viewEntity()))
            continue;
        ImDrawList* draw = ImGui::GetForegroundDrawList();
        ImVec2 position = {0,0};
        std::string text = std::to_string(node.data.value);
        float fontSize = 18.0f * editorState.windowData.zoom;
        ImVec2 textSize = editorState.MonoFontBold->CalcTextSizeA(fontSize, FLT_MAX, 0.f, text.c_str());
        position = worldToImGui<ImVec2>(api.getInstance(node.viewEntity()).position, editorState.windowData);
        position.x -= textSize.x * .5f;
        position.y -= textSize.y * .5f;
        draw->AddText(editorState.MonoFontBold, fontSize, position, IM_COL32_BLACK, text.c_str());
    }
    ImGui::SetNextWindowPos({0,0}, ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(260, 140), ImGuiCond_Once);
    ImGui::Begin("debug");
    ImGui::PushID(-1);
    ImGui::Text("Current State: %s", stateMachine.stateToString().c_str());
    ImGui::Text("Current Entity: %i", editorState.holdEntity.index);
    ImGui::Text("Current Zoom: %f", editorState.windowData.zoom);
    ImGui::PopID();
    ImGui::End();
    
    for(auto& [e, open] : editorState.openedWindows){
        if(editorState.openedWindows.contains(e) && editorState.openedWindows[e]){
            ImGui::SetNextWindowPos(ImGui::GetMousePos(), ImGuiCond_Appearing);
            ImGui::SetNextWindowSize(ImVec2(250, 150), ImGuiCond_Once);
            ImGui::Begin( ("Circle " + std::to_string(e.index)).c_str(), &editorState.openedWindows[e]);
            ImGui::PushID(e.index);

            if(ImGui::Button("Delete")){
                graph.deleteNode(e);
                graph.update();
                editorState.openedWindows[e] = false;
            }

            ImGui::PopID();
            ImGui::End();
        }
    }
    float scroll = ImGui::GetIO().MouseWheel;
    
    glm::vec2 pos = mousePosition(editorState.windowData);
    if(scroll != 0){
        scroll /= 20;
        float oldZoom = editorState.windowData.zoom;
        editorState.windowData.zoom *= 1 + scroll;
        if(editorState.windowData.zoom < .05){
            editorState.windowData.zoom = .05;
        } else if(editorState.windowData.zoom > 120){
            editorState.windowData.zoom = 120;
        } else {
            editorState.windowData.offset += (pos - editorState.windowData.offset) * (1.f - (oldZoom / editorState.windowData.zoom));
            api.setZoom(editorState.windowData.zoom);
            api.setOffset(editorState.windowData.offset);
        }
    }
}

int main(){
    ThING::API api(ApiFlags_UpdateCallbackFirst);
    editorState.graph = new Graph(api);
    stateMachine.bind(api, editorState);
    api.setBackgroundColor(Style::Color::Background);
    api.setUpdateCallback(updateCallback);
    api.setUICallback(uiCallback);
    api.run();
}
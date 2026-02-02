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
        editorState.graph->addNode({0,0});
        editorState.graph->viewNodeList().back().data.tags.noUpdate = true;
        editorState.graph->viewNodeList().back().data.tags.noMove = true;
        editorState.graph->viewNodeList().back().data.tags.noDelete = true;
    }

    if(editorState.config.lineForces)
        graph.applyLineForces(forces);
    if(editorState.config.nodeRepulsion)
        graph.applyNodeRepulsion(forces);
    if(editorState.config.centerAttraction)
        graph.applyCenterAttraction(forces);
    graph.update();

    // if(!(ImGui::GetFrameCount() % 4)){
    //     for (Node& node : graph.viewNodeList()) {
    //         if(node.data.delay < 10){
    //             for(Link& link : node.links){
    //                 api.getLine(link.viewLine()).color = Style::Color::Line;
    //             }
    //             node.data.delay++;
    //             continue;
    //         }
    //         node.data.delay = 0;
    //         for (Link& link : node.links) {
    //             if (node.data.value > 0) {
    //                 api.getLine(link.viewLine()).color = Style::Color::TempLine;
    //                 node.data.value--;
    //                 graph.getNode(link.viewConnection().index).data.value++;
    //             }
    //         }
    //     }
    // }

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
        if(!api.exists(node.viewEntity()) || !node.data.value)
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
    ImGui::PushFont(editorState.MonoFont, Style::UI::TextSize);
    ImGui::SetNextWindowPos({0,0}, ImGuiCond_Once);
    ImGui::SetNextWindowCollapsed(true, ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(320, 400), ImGuiCond_Once);
    beginWindow(api, "debug");
    ImGui::PushID(-1);
    ImGui::Text("Current State: %s", stateMachine.stateToString().c_str());
    ImGui::Text("Current Entity: %i", editorState.holdEntity.index);
    ImGui::Text("Current Zoom: %f", editorState.windowData.zoom);
    ImGui::Text("Current Points: %i", editorState.game.points);
    {
        if(openTree(api, "Update Configuration")){
            ImGui::PushItemWidth(120.0f);
            ImGui::SliderFloat("Center Attraction", &forces.centerAttraction, 0, .1f);
            ImGui::SliderFloat("Node Repulsion", &forces.nodeRepulsionForce, 0, 10.f);
            ImGui::SliderFloat("Repulsion Radius", &forces.nodeRepulsionRadius, 0, 300.f);
            ImGui::SliderFloat("Line Force", &forces.lineForce, 0, .2f);
            ImGui::SliderFloat("Line Center", &forces.lineCenter, 0, 180.f);
            ImGui::PopItemWidth();
            !forces.centerAttraction ? editorState.config.centerAttraction = false : editorState.config.centerAttraction = true;
            !forces.nodeRepulsionForce ? editorState.config.nodeRepulsion = false : editorState.config.nodeRepulsion = true;
            !forces.lineForce ? editorState.config.lineForces = false : editorState.config.lineForces = true;

            ImGui::TreePop();
}
    }
    ImGui::PopID();
    ImGui::End();
    
    for(auto& [e, open] : editorState.openedWindows){
        if(editorState.openedWindows.contains(e) && editorState.openedWindows[e]){
            ImGui::SetNextWindowPos(ImGui::GetMousePos(), ImGuiCond_Appearing);
            ImGui::SetNextWindowSize(ImVec2(250, 250), ImGuiCond_Once);
            beginWindow(api, ("Circle " + std::to_string(e.index)).c_str(), &editorState.openedWindows[e]);
            ImGui::PushID(e.index);
            if (!editorState.openedWindows[e]){
                api.playAudio(Style::Audio::CloseUi, 180);
            }
            if(openTree(api, "Tags")){
                Tags& tags = graph.getNode(e.index).data.tags;
                int lastTags = tags.noDelete + tags.noUpdate + tags.noMove;
                ImGui::Checkbox("No Delete", &tags.noDelete);
                ImGui::Checkbox("No Update", &tags.noUpdate);
                ImGui::Checkbox("No Move", &tags.noMove);
                int newTags = tags.noDelete + tags.noUpdate + tags.noMove;
                if(newTags < lastTags){
                    api.playAudio(Style::Audio::UnCheck);
                }
                if(newTags > lastTags){
                    api.playAudio(Style::Audio::Check);
                }

                ImGui::TreePop();
            }
            
            if(!graph.getNode(e.index).data.tags.noDelete){
                if(ImGui::Button("Delete")){
                    api.playAudio(Style::Audio::DeleteNode);
                    graph.deleteNode(e);
                    graph.update();
                    editorState.openedWindows[e] = false;
                }
            }
            ImGui::PopID();
            ImGui::End();
        }
    }
    ImGui::PopFont();
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
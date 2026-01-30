#include "ThING/core.h"
#include "ThING/types/apiTypes.h"
#include "ThING/types/enums.h"
#include "ThING/types/renderData.h"
#include "auxiliar/window.h"
#include "glm/fwd.hpp"
#include "globals.h"
#include <ThING/api.h>
#include <imgui.h>

#include "auxiliar/style.h"
#include "graph/graph.h"

void updateCallback(ThING::API& api, FPSCounter& fps){
    static Graph& graph = *editorState.graph; 
    static bool first = true;

    static Entity tempLine = INVALID_ENTITY;
    api.getWindowSize(&editorState.windowData.size.width, &editorState.windowData.size.height);

    if(first){
        ApplyNodeEditorStyle();
        first = false;
    }
    ImGuiIO& io = ImGui::GetIO();

    if(!io.WantCaptureMouse){
        switch (editorState.stateM) {
            case StateM::Idle:
                editorState.holdEntity = INVALID_ENTITY;
                if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)){
                    editorState.holdEntity = hitEntity(api, editorState.windowData);
                    editorState.stateM = StateM::WaitLeftIdle;
                }
                if(ImGui::IsMouseClicked(ImGuiMouseButton_Right)){
                    editorState.holdEntity = hitEntity(api, editorState.windowData);
                    editorState.stateM = StateM::WaitRightIdle;
                }
                break;
            case StateM::WaitLeftIdle:
                if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)){
                    if(editorState.holdEntity != INVALID_ENTITY){
                        editorState.stateM = StateM::DraggingLine;
                        break;
                    }
                    if(hitEntity(api, editorState.windowData, Style::NodeSize + Style::OutlineWidth) == INVALID_ENTITY){
                        Entity e = graph.addNode(mousePosition(editorState.windowData));
                    }
                    editorState.stateM = StateM::Idle;
                    break;
                }
                if(ImGui::IsMouseDragging(ImGuiMouseButton_Left, Style::LockDragging)){
                    if(editorState.holdEntity == INVALID_ENTITY){
                        editorState.stateM = StateM::PanningCamera;
                        break;
                    }
                    editorState.stateM = StateM::DraggingLine;
                }
                break;
            case StateM::WaitRightIdle:
                if(ImGui::IsMouseReleased(ImGuiMouseButton_Right)){
                    if(editorState.holdEntity != INVALID_ENTITY){
                        editorState.openedWindows[editorState.holdEntity] = true;
                    }
                    editorState.stateM = StateM::Idle;
                    break;
                }
                if(ImGui::IsMouseDragging(ImGuiMouseButton_Right)){
                    editorState.stateM = StateM::DraggingNode;
                }
                break;
            case StateM::DraggingLine:
                if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)){
                    if(tempLine != INVALID_ENTITY){
                        api.deleteInstance(tempLine);
                        tempLine = INVALID_ENTITY;
                    }
                    if(api.exists(editorState.holdEntity)){
                        graph.connect(editorState.holdEntity, hitEntity(api, editorState.windowData));
                    }
                    editorState.stateM = StateM::Idle;
                    break;
                }
                if(tempLine == INVALID_ENTITY){
                    tempLine = api.addLine(api.getInstance(editorState.holdEntity).position, 
                        mousePosition(editorState.windowData), Style::LineWidth);
                    api.getLine(tempLine).color = Style::Color::TempLine;
                    api.getLine(tempLine).outlineColor = Style::Color::Outline;
                    api.getLine(tempLine).outlineSize = Style::OutlineWidth;
                    api.getLine(tempLine).objectID = 1;
                } else {
                    api.getLine(tempLine).point2 = mousePosition(editorState.windowData);
                }
                api.updateOutlines();
                break;
            case StateM::DraggingNode:
                if(ImGui::IsMouseReleased(ImGuiMouseButton_Right)){
                    editorState.stateM = StateM::Idle;
                }
                if(editorState.holdEntity != INVALID_ENTITY){
                    api.getInstance(editorState.holdEntity).position = mousePosition(editorState.windowData);
                    graph.update();
                }
                break;
            case StateM::PanningCamera:
                if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)){
                    editorState.stateM = StateM::Idle;
                    break;
                }
                ImVec2 delta = ImGui::GetIO().MouseDelta;
                editorState.windowData.offset.x -= delta.x / editorState.windowData.zoom;
                editorState.windowData.offset.y -= delta.y / editorState.windowData.zoom;
                api.setOffset(editorState.windowData.offset);
                break;
        }
    }
}

void uiCallback(ThING::API& api, FPSCounter& fps){
    static Graph& graph = *editorState.graph;
    ImGui::SetNextWindowPos({0,0}, ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(260, 80), ImGuiCond_Once);
    ImGui::Begin("debug");
    ImGui::PushID(-1);
    ImGui::Text("Current State: %s", StateMToString(editorState.stateM));
    ImGui::Text("Current State: %i", editorState.holdEntity.index);
    ImGui::PopID();
    ImGui::End();
    std::span<InstanceData> circleInstances = api.getInstanceVector(InstanceType::Circle);
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
        editorState.windowData.offset += (pos - editorState.windowData.offset) * (1.f - (oldZoom / editorState.windowData.zoom));
        api.setZoom(editorState.windowData.zoom);
        api.setOffset(editorState.windowData.offset);
    }
}

int main(){
    ThING::API api;
    editorState.graph = new Graph(api);
    api.setBackgroundColor(Style::Color::Background);
    api.setUpdateCallback(updateCallback);
    api.setUICallback(uiCallback);
    api.run();
}
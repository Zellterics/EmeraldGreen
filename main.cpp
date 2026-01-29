#include "ThING/core.h"
#include "ThING/types/apiTypes.h"
#include "ThING/types/enums.h"
#include "ThING/types/renderData.h"
#include "glm/fwd.hpp"
#include "globals.h"
#include <ThING/api.h>
#include <imgui.h>

#include "auxiliar/style.h"
#include "graph/graph.h"

void updateCallback(ThING::API& api, FPSCounter fps){
    static Graph& graph = *editorState.graph; 
    static bool first = true;

    static Entity tempLine = INVALID_ENTITY;
    api.getWindowSize(&editorState.windowData.size.width, &editorState.windowData.size.height);

    if(first){
        ApplyNodeEditorStyle();
        first = false;
    }

    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureMouse){
        if(ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !(editorState.rightHold || editorState.rightDrag)){
            editorState.holdEntity = hitEntity(api, editorState.windowData);
            editorState.leftHold = true;
        }
        if(ImGui::IsMouseDragging(ImGuiMouseButton_Left) && editorState.leftHold){
            if(editorState.holdEntity == INVALID_ENTITY){
                
                ImVec2 delta = ImGui::GetIO().MouseDelta;
                editorState.windowData.offset.x -= delta.x / editorState.windowData.zoom;
                editorState.windowData.offset.y -= delta.y / editorState.windowData.zoom;
                api.setOffset(editorState.windowData.offset);
            } else {
                if(tempLine == INVALID_ENTITY){
                    tempLine = api.addLine(api.getInstance(editorState.holdEntity).position, mousePosition(editorState.windowData), Color::LineWidth);
                    api.getLine(tempLine).color = Color::Line;
                    api.getLine(tempLine).outlineColor = Color::Outline;
                    api.getLine(tempLine).outlineSize = Color::OutlineWidth;
                    api.getLine(tempLine).objectID = 1;
                } else {
                    api.getLine(tempLine).point2 = mousePosition(editorState.windowData);
                }
            }
            editorState.leftDrag = true;
        }
        if(ImGui::IsMouseReleased(ImGuiMouseButton_Left) && editorState.leftHold){
            if(editorState.leftDrag){
                if(api.exists(editorState.holdEntity)){
                    graph.connect(editorState.holdEntity, hitEntity(api, editorState.windowData));
                }
            } else {
                Entity e = graph.addNode(mousePosition(editorState.windowData));
            }
            if(tempLine != INVALID_ENTITY){
                api.deleteInstance(tempLine);
                tempLine = INVALID_ENTITY;
            }
            editorState.leftDrag = false;
            editorState.leftHold = false;
        }
        if(!(editorState.leftDrag || editorState.leftHold)){
            if(ImGui::IsMouseClicked(ImGuiMouseButton_Right)){
                editorState.holdEntity = hitEntity(api, editorState.windowData);
                editorState.rightHold = true;
            }
        }
        if(ImGui::IsMouseDragging(ImGuiMouseButton_Right) && editorState.rightHold){
            if(editorState.holdEntity != INVALID_ENTITY){
                api.getInstance(editorState.holdEntity).position = mousePosition(editorState.windowData);
                graph.update();
            }
            editorState.rightDrag = true;
        }
        if(ImGui::IsMouseReleased(ImGuiMouseButton_Right) && editorState.rightHold){
            
            if(!editorState.rightDrag && editorState.holdEntity != INVALID_ENTITY){
                editorState.openedWindows[editorState.holdEntity] = true;
            }
            editorState.rightHold = false;
            editorState.rightDrag = false;
        }
    }
}

void uiCallback(ThING::API& api, FPSCounter fps){
    static Graph& graph = *editorState.graph; 
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
    api.setBackgroundColor(Color::Background);
    api.setUpdateCallback(updateCallback);
    api.setUICallback(uiCallback);
    api.run();
}
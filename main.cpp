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

#include "external/Monocraft.h"
#include "external/Monocraft-Bold.h"

void updateCallback(ThING::API& api, FPSCounter& fps){
    static Graph& graph = *editorState.graph; 
    static bool first = true;

    static Entity tempLine = INVALID_ENTITY;
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
    ImGuiIO& io = ImGui::GetIO();
    static Node* tempNode = nullptr;
    static std::unordered_map<Node*, int> visited;
    std::span<Node> nodes = graph.viewNodeList();

    for(Node& node : nodes){
        glm::vec2& nodePosition = api.getInstance(node.viewEntity()).position;
        for(Link& link : node.links){
            glm::vec2& pos1 = api.getLine(link.viewLine()).point1;
            glm::vec2& pos2 = api.getLine(link.viewLine()).point2;
            float distance = length(pos2 - pos1);
            glm::vec2 direction = normalize(pos2 - pos1);
            glm::vec2 force = direction * (distance - 90.f) * 0.04f;
            nodePosition += force;
            api.getInstance(link.viewConnection()).position -= force;
        }
    }
    
    std::unordered_map<int64_t, std::vector<Entity>> hashGrid;
    hashGrid.reserve(nodes.size());
    float size = Style::NodeSize * 10;
    for(Node& node : nodes){
        glm::vec2& nodePosition = api.getInstance(node.viewEntity()).position;
        hashGrid[(int64_t(floor(nodePosition.x / size)) << 32) | (uint32_t(floor(nodePosition.y / size)))].push_back(node.viewEntity());
    }
    for(Node& node : nodes){
        glm::vec2& nodePosition = api.getInstance(node.viewEntity()).position;
        int64_t nodeKey = (int64_t(floor(nodePosition.x / size)) << 32) | (uint32_t(floor(nodePosition.y / size)));
        int xKey = int(floor(nodePosition.x / size));
        int yKey = int(floor(nodePosition.y / size));

        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                int nx = xKey + i;
                int ny = yKey + j;

                int64_t key = (int64_t(nx) << 32) | (uint32_t(ny));

                auto it = hashGrid.find(key);
                if (it == hashGrid.end())
                    continue;

                for (Entity& other : it->second) {
                    if (other == node.viewEntity())
                        continue;
                    glm::vec2 otherPos = api.getInstance(other).position;
                    glm::vec2 delta = nodePosition - otherPos;
                    float dist2 = dot(delta, delta);

                    float minDist = size;
                    float minDist2 = minDist * minDist;

                    if (dist2 > 0.0001f && dist2 < minDist2) {

                        float dist = sqrt(dist2);
                        glm::vec2 dir = delta / dist;

                        float t = (minDist - dist) / minDist;
                        t = std::clamp(t, 0.f, 1.f);

                        nodePosition += dir * t * 4.5f;
                    }
                }
            }
        }
    }
    for(Node& node : nodes){
        api.getInstance(node.viewEntity()).position *= .99f;
    }
    graph.update();

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
                if(ImGui::IsMouseClicked(ImGuiMouseButton_Middle)){
                    for(Node& node : graph.viewNodeList()){
                        if(api.exists(node.viewEntity())){
                            editorState.stateM = StateM::PlayingAnimation;
                            tempNode = &node;
                            visited.clear();
                            visited[tempNode] = 0;
                            break;
                        }
                    }
                }
                break;
            case StateM::WaitLeftIdle:
                if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)){
                    static int i = 0;
                    if(editorState.holdEntity != INVALID_ENTITY){
                        editorState.stateM = StateM::DraggingLine;
                        break;
                    }
                    if(hitEntity(api, editorState.windowData, Style::NodeSize + Style::OutlineWidth) == INVALID_ENTITY){
                        Entity e = graph.addNode(mousePosition(editorState.windowData));
                        graph.getNode(e.index).value = i++;
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
                    api.getLine(tempLine).point1 = api.getInstance(editorState.holdEntity).position;
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
                {
                    ImVec2 delta = ImGui::GetIO().MouseDelta;
                    editorState.windowData.offset.x -= delta.x / editorState.windowData.zoom;
                    editorState.windowData.offset.y -= delta.y / editorState.windowData.zoom;
                    api.setOffset(editorState.windowData.offset);
                }
                break;
            case StateM::PlayingAnimation:
                if(ImGui::IsMouseClicked(ImGuiMouseButton_Right)){
                    for(LineData& line : api.getLineVector()){
                        if (!line.alive){
                            continue;
                        }
                        line.color = Style::Color::Line;
                        line.outlineColor = Style::Color::Outline;
                        line.objectID = 1;
                        api.updateOutlines();
                    }
                    editorState.stateM = StateM::WaitRightIdle;
                }
                if(ImGui::GetFrameCount() % 10){
                    return;
                }
                if(!visited.contains(tempNode)){
                    visited[tempNode] = 0;
                }
                if(visited[tempNode] < tempNode->links.size()){
                    Entity lineE = tempNode->links[visited[tempNode]].viewLine();
                    api.getLine(lineE).color *= .8f;
                    api.getLine(lineE).color.a = 1;
                    api.getLine(lineE).objectID = 2;
                    api.updateOutlines();
                    Entity e = tempNode->links[visited[tempNode]].viewConnection();
                    visited[tempNode]++;
                    tempNode = &graph.getNode(e.index);
                } else if (tempNode->links.size() == 0){
                    for(LineData& line : api.getLineVector()){
                        if (!line.alive){
                            continue;
                        }
                        line.color = Style::Color::Line;
                        line.outlineColor = Style::Color::Outline;
                        line.objectID = 1;
                        api.updateOutlines();
                    }
                    editorState.stateM = StateM::Idle;
                } else {
                    visited[tempNode] = 0;
                    Entity lineE = tempNode->links[visited[tempNode]].viewLine();
                    api.getLine(lineE).color *= .8f;
                    api.getLine(lineE).color.a = 1;
                    api.getLine(lineE).objectID = 2;
                    api.updateOutlines();
                    Entity e = tempNode->links[visited[tempNode]].viewConnection();
                    visited[tempNode]++;
                    tempNode = &graph.getNode(e.index);
                }
            break;
        }
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
        std::string text = std::to_string(node.value);
        float fontSize = 18.0f * editorState.windowData.zoom;
        ImVec2 textSize = editorState.MonoFontBold->CalcTextSizeA(fontSize, FLT_MAX, 0.f, text.c_str());
        position = worldToImGui<ImVec2>(api.getInstance(node.viewEntity()).position, editorState.windowData);
        position.x -= textSize.x * .5f;
        position.y -= textSize.y * .5f;
        draw->AddText(editorState.MonoFontBold, fontSize, position, IM_COL32_BLACK, text.c_str());
    }
    ImGui::SetNextWindowPos({0,0}, ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(260, 80), ImGuiCond_Once);
    ImGui::Begin("debug");
    ImGui::PushID(-1);
    ImGui::Text("Current State: %s", StateMToString(editorState.stateM));
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
    api.setBackgroundColor(Style::Color::Background);
    api.setUpdateCallback(updateCallback);
    api.setUICallback(uiCallback);
    api.run();
}
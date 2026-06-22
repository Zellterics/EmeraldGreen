#include "ThING/types/apiTypes.h"
#include "ThING/types/enums.h"
#include "auxiliar/style.h"
#include "glm/fwd.hpp"
#include "globals.h"
#include <ThING/api.h>
#include <cstdint>
#include <string>

#include "graph/node.h"
#include "imgui.h"
#include "auxiliar/editor.h"

#include "external/Monocraft.h"
#include "external/Monocraft-Bold.h"
#include "stateMachine/stateMachine.h"

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
        // loadLevel(api, "Menu");
    }
    if(!api.exists(editorState.game.currentNode)){
        editorState.game.currentNode = INVALID_ENTITY;
    }
    if(graph.viewNodeList().size() > 0){
        if(stateMachine.getState() == StateM::Idle){
            if(editorState.game.currentNode != INVALID_ENTITY){
                graph.getNode(editorState.game.currentNode).setType(NodeType::None);
                api.getInstance(editorState.game.currentNode).color = graph.getNode(editorState.game.currentNode).data.baseColor;
            }
            for(Node& node : graph.viewNodeList()){
                if(api.getInstance(node.viewEntity()).alive){
                    editorState.game.currentNode = node.viewEntity();
                    graph.getNode(editorState.game.currentNode).setType(NodeType::None);
                    api.getInstance(editorState.game.currentNode).color = graph.getNode(editorState.game.currentNode).data.baseColor;
                    break;
                }
            }
        } else if(editorState.game.currentNode == INVALID_ENTITY){
            for(Node& node : graph.viewNodeList()){
                if(api.getInstance(node.viewEntity()).alive){
                    editorState.game.currentNode = node.viewEntity();
                    break;
                }
            }
        }

    }
    if(editorState.config.lineForces)
        graph.applyLineForces(forces);
    if(editorState.config.nodeRepulsion)
        graph.applyNodeRepulsion(forces);
    if(editorState.config.centerAttraction)
        graph.applyCenterAttraction(forces);
    graph.update();

    for(Node& node : graph.viewNodeList()){
        if(node.data.type == NodeType::Goal){
            node.data.value = editorState.game.points;
        }
    }
    static float secondCounter = 0;
    if(editorState.game.currentNode != INVALID_ENTITY){
        if(graph.getNode(editorState.game.currentNode).data.type == NodeType::Goal && editorState.game.winTimer < 0 && editorState.game.loseTimer < 0){
            Node& winNode = graph.getNode(editorState.game.currentNode);
            if(winNode.data.value >= 0){
                api.playAudio(Style::Audio::Win);
                secondCounter = 0;
                editorState.game.winTimer = 1;
            } else {
                api.playAudio(Style::Audio::Lose);
                secondCounter = 0;
                editorState.game.loseTimer = 1;
            }
        }
    }
    if(editorState.game.winTimer > 0){
        secondCounter += fps.getDeltaTime();
        if(secondCounter >= 1){
            secondCounter = 0;
            editorState.game.winTimer--;
            if(editorState.game.winTimer <= 0){
                stateMachine.getState() = StateM::MenuIdle;
                editorState.game.currentNode = INVALID_ENTITY;
                editorState.game.winTimer = -1;
                loadLevel(api, "Menu");
            }
        }
        return;
    }
    if(editorState.game.loseTimer > 0){
        secondCounter += fps.getDeltaTime();
        if(secondCounter >= 1){
            secondCounter = 0;
            editorState.game.loseTimer--;
            if(editorState.game.loseTimer <= 0){
                loadLevel(api, editorState.game.gameLevelName);
                stateMachine.getState() = StateM::GameIdle;
                editorState.game.points = 0;
                editorState.game.currentNode = INVALID_ENTITY;
                editorState.game.loseTimer = -1;
            }
        }
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    if(!io.WantCaptureMouse){
        stateMachine.update();
    }{
        static Entity lastNode = editorState.game.currentNode;
        if(editorState.game.currentNode != INVALID_ENTITY){
            if(lastNode == INVALID_ENTITY){
                lastNode = editorState.game.currentNode;
            }
            if(lastNode != editorState.game.currentNode){
                lastNode = editorState.game.currentNode;
                switch (graph.getNode(lastNode).data.type) {
                    case NodeType::Bad:
                        editorState.game.points--;
                        break;
                    case NodeType::None:
                        break;
                    case NodeType::Good:
                        editorState.game.points++;
                        graph.getNode(lastNode).data.type = NodeType::None;
                        graph.getNode(lastNode).data.baseColor = Style::Color::Node;
                        graph.getNode(lastNode).data.selectedColor = Style::Color::NodeSelected;
                        api.getInstance(lastNode).color = Style::Color::NodeSelected;
                        break;
                    case NodeType::Goal:
                        //editorState.game.points += 100;
                        break;
                    case NodeType::Count:
                        break;
                }
            }
            api.getInstance(editorState.game.currentNode).color = graph.getNode(editorState.game.currentNode).data.selectedColor;
            glm::vec2 size = {Style::NodeSize + Style::NodeSelectedPadding, Style::NodeSize + Style::NodeSelectedPadding};
            api.getInstance(editorState.game.currentNode).scale = size;
        } else {
            lastNode = INVALID_ENTITY;
        }
    }
}

void uiCallback(ThING::API& api, FPSCounter& fps){
    static Graph& graph = *editorState.graph;
    std::span<InstanceData> circleInstances = api.getInstanceVector(InstanceType::Circle);
    std::span<Node> nodes = graph.viewNodeList();
    for(Node& node : nodes){
        if(!api.exists(node.viewEntity()) || !node.data.value)
            continue;
        ImDrawList* draw = ImGui::GetBackgroundDrawList();
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
    if(stateMachine.getState() == StateM::Menu){
        menuWindow(api);
    } else {
        debugWindow(api);
        nodeWindows(api);
    }
    scrollZoom(api);    
    ImGui::PopFont();
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
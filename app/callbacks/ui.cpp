#include "ui.h"
#include "../globals.h"
#include "../auxiliar/persistence.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "../auxiliar/style.h"
#include <string>

#include "../../external/Monocraft.h"
#include "../../external/Monocraft-Bold.h"

void uiCallback(ThING::API& api, FPSCounter& fps){
    drawNodeValues(api);
    if(stateMachine.getState() == StateM::Menu){
        menuWindow(api);
    } else {
        debugWindow(api);
        nodeWindows(api);
    }
    ImGuiIO& io = ImGui::GetIO();
    if(!io.WantCaptureMouse){
        scrollZoom(api);
    }  
}

void imGuiInitialize(){
    ImGuiIO& io = ImGui::GetIO();
    ImFontConfig cfg;
    cfg.FontDataOwnedByAtlas = false;
    editorState.MonoFontSize = 18.f;
    editorState.MonoFont = io.Fonts->AddFontFromMemoryTTF(const_cast<uint8_t*>(Monocraft), MonocraftSize, 18.f, &cfg);
    editorState.MonoFontBold = io.Fonts->AddFontFromMemoryTTF(const_cast<uint8_t*>(Monocraft_Bold), Monocraft_BoldSize, 18.f, &cfg);
    ApplyNodeEditorStyle();
    zlog.debug("imGui Initialized");
}

void nodeWindows(ThING::API& api){
    Graph& graph = *editorState.graph;
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
                Tags& tags = graph.getNode(e).data.tags;
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
            if(openTree(api, "Type")){
                int lastChoice = 0;
                int newChoice = 0;
                newChoice = static_cast<int>(graph.getNode(e).data.type);
                lastChoice = static_cast<int>(graph.getNode(e).data.type);
                ImGui::RadioButton("None", &newChoice, 0);
                ImGui::RadioButton("Bad", &newChoice, 1);
                ImGui::RadioButton("Good", &newChoice, 2);
                ImGui::RadioButton("Goal", &newChoice, 3);
                ImGui::RadioButton("Switch", &newChoice, 4);
                ImGui::RadioButton("Start", &newChoice, 5);
                if(lastChoice != newChoice){
                    if(lastChoice == static_cast<int>(NodeType::Start)){
                        api.playAudio(Style::Audio::UnCheck);
                    } else {
                        if(newChoice < lastChoice){
                            api.playAudio(Style::Audio::UnCheck);
                        }
                        if(newChoice > lastChoice){
                            api.playAudio(Style::Audio::Check);
                        }
                        graph.getNode(e).setType(static_cast<NodeType>(newChoice));
                        if(newChoice == static_cast<int>(NodeType::Start)){
                            graph.getNode(editorState.game.currentNode).setType(NodeType::None);
                            api.getInstance(editorState.game.currentNode).color = Style::Color::Node;
                            editorState.game.currentNode = e;
                            api.getInstance(e).color = graph.getNode(e).data.selectedColor;
                        } else {
                            api.getInstance(e).color = graph.getNode(e).data.baseColor;
                        }
                        
                        
                    }
                }
                ImGui::TreePop();
            }
            
            if(openTree(api, "Data")){
                int lastData = 0;
                int newData = 0;
                newData = graph.getNode(e).data.value;
                lastData = newData;
                if(ImGui::InputInt("Value", &graph.getNode(e).data.value)){
                    if(graph.getNode(e).data.value > 999){
                        graph.getNode(e).data.value = 999;
                        api.playAudio(Style::Audio::DisconnectNode);
                        ImGui::ClearActiveID();
                    }
                    if(graph.getNode(e).data.value < 0){
                        graph.getNode(e).data.value = 0;
                        api.playAudio(Style::Audio::DisconnectNode);
                        ImGui::ClearActiveID();
                    }
                }
                if(openTree(api, "Lines")){
                    for(Link& link : graph.getNode(e).links){
                        bool state = link.active;
                        ImGui::Checkbox(("Line " + std::to_string(link.viewLine().index)).c_str(), &link.active);
                        if(state > link.active){
                            api.getInstance(link.viewLine()).color = Style::Color::InnactiveLine;
                            api.playAudio(Style::Audio::UnCheck);
                        }
                        if(state < link.active){
                            api.getInstance(link.viewLine()).color = Style::Color::Line;
                            api.playAudio(Style::Audio::Check);
                        }

                    }
                    ImGui::TreePop();
                }
                ImGui::TreePop();
            }

            if(!graph.getNode(e.index).data.tags.noDelete){
                if(ImGui::Button("Delete")){
                    {
                        api.playAudio(Style::Audio::DeleteNode);
                        graph.deleteNode(e);
                        graph.update();
                        editorState.openedWindows[e] = false;
                    }
                }
            }
            ImGui::PopID();
            ImGui::End();
        }
    }
}

void scrollZoom(ThING::API &api){
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



void debugWindow(ThING::API &api){
    ImGui::SetNextWindowPos({0,0}, ImGuiCond_Once);
    ImGui::SetNextWindowCollapsed(true, ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(320, 500), ImGuiCond_Once);
    beginWindow(api, "debug");
    ImGui::PushID(-1);
    ImGui::Text("Current State: %s", stateMachine.stateToString().c_str());
    ImGui::Text("Current Entity: %i", editorState.holdEntity.index);
    ImGui::Text("LoseTimer: %i", editorState.game.loseTimer);
    ImGui::Text("Current Points: %i", editorState.game.points);
    static int volume = 255;
    ImGui::SliderInt("Volume", &volume, 0, 255);
    api.setVolume(volume);
    if(openTree(api, "Update Configuration")){
        ImGui::PushItemWidth(120.0f);
        ImGui::SliderFloat("Center Attraction", &forces.centerAttraction, 0, .1f);
        ImGui::SliderFloat("Node Repulsion", &forces.nodeRepulsionForce, 0, 10.f);
        ImGui::SliderFloat("Repulsion Radius", &forces.nodeRepulsionRadius, 0, 300.f);
        ImGui::SliderFloat("Line Force", &forces.lineForce, 0, .2f);
        ImGui::SliderFloat("Line Center", &forces.lineCenter, 0, 180.f);
        ImGui::PopItemWidth();
        editorState.config.forceFlags = 
            (forces.centerAttraction ? 0 : ForceFlags_SkipCenterAttraction) |
            (forces.nodeRepulsionForce ? 0 : ForceFlags_SkipNodeRepulsion) |
            (forces.lineForce ? 0 : ForceFlags_SkipLines);

        ImGui::TreePop();
    }
    static char levelName[20] = "Untitled";
    ImGui::InputText("Level Name", levelName, 20);
    if(ImGui::Button("save")){
        saveLevel(api, levelName);
    }
    ImGui::SameLine(0, 12.f);
    if(ImGui::Button("load")){
        loadLevel(api, levelName);
        stateMachine.getState() = StateM::GameIdle;
    }
    ImGui::PopID();
    ImGui::End();
}

void menuWindow(ThING::API &api){
    static bool firstFrame = true;
    ImGuiIO& io = ImGui::GetIO();

    ImGui::SetNextWindowBgAlpha(0);
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("MainMenu",
        nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus
    );
    ImVec2 buttonSize = {200, 40};
    ImGuiStyle& style = ImGui::GetStyle();

    float totalHeight =
        3 * buttonSize.y +
        2 * style.ItemSpacing.y;

    float startY = (ImGui::GetWindowSize().y - totalHeight) * 0.5f;
    ImGui::SetCursorPosY(startY);

    auto centerX = [&]() {
        ImGui::SetCursorPosX(
            (ImGui::GetWindowSize().x - buttonSize.x) * 0.5f
        );
    };

    centerX();
    if (ImGui::Button("Resume", buttonSize) || ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
        if(!firstFrame){
            firstFrame = true;
            stateMachine.getState() = stateMachine.getReturnState();
        }else{
            firstFrame = false;
        }
    }

    centerX();
    if (ImGui::Button("Menu", buttonSize)) {
        editorState.game.level = -1;
        loadMenu(api);
        firstFrame = true;
        stateMachine.getState() = StateM::MenuIdle;
    }

    centerX();
    std::string exitLabel = stateMachine.getReturnState() == StateM::MenuIdle ? "Save and Exit" : "Exit";
    if (ImGui::Button(exitLabel.c_str(), buttonSize)) {
        if(stateMachine.getReturnState() == StateM::MenuIdle){
            saveLevel(api, "Menu");
        }
        api.EXIT();
    }


    ImGui::Text("ASDADSSDDASSAD");
    ImGui::End();
}

void drawNodeValues(ThING::API& api){
    std::span<Node> nodes = editorState.graph->viewNodeList();
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
}
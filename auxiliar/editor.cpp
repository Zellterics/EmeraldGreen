#include "editor.h"
#include "../globals.h"
#include "../external/json.hpp"
#include "ThING/types/enums.h"
#include "imgui.h"
#include "style.h"
#include <cstdint>
#include <string>

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
                if(newChoice < lastChoice){
                    api.playAudio(Style::Audio::UnCheck);
                }
                if(newChoice > lastChoice){
                    api.playAudio(Style::Audio::Check);
                }
                if(!(editorState.game.currentNode == e)){
                    graph.getNode(e).setType(static_cast<NodeType>(newChoice));
                    api.getInstance(e).color = graph.getNode(e).data.baseColor;
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

using json = nlohmann::json;
void saveLevel(ThING::API& api, std::string filename){
    Graph& graph = *editorState.graph;
    json level;
    for(Node& node : graph.viewNodeList()){
        json jNode;
        jNode["Type"] = node.data.type;
        jNode["ID"] = node.viewEntity().index;
        jNode["Value"] = node.data.value;
        jNode["Alive"] = api.getInstance(node.viewEntity()).alive;
        jNode["Position"]["x"] = api.getInstance(node.viewEntity()).position.x;
        jNode["Position"]["y"] = api.getInstance(node.viewEntity()).position.y;
        jNode["Tags"]["noDelete"] = node.data.tags.noDelete;
        jNode["Tags"]["noMove"] = node.data.tags.noMove;
        jNode["Tags"]["noUpdate"] = node.data.tags.noUpdate;
        for(Link& link : node.links){
            jNode["Links"].push_back(link.viewConnection().index);
        }
        level["nodes"].push_back(jNode);
    }
    level["NodeRepulsionForce"] = forces.nodeRepulsionForce;
    level["NodeRepulsionRadius"] = forces.nodeRepulsionRadius;
    level["LineForce"] = forces.lineForce;
    level["LineCenter"] = forces.lineCenter;
    level["CenterAttraction"] = forces.centerAttraction;
    std::ofstream file(filename + ".json");
    file << level.dump(4);
    file.close();
}

void loadLevel(ThING::API& api, std::string filename){
    Graph& graph = *editorState.graph;
    std::ifstream file(filename + ".json");
    if (!file.is_open()) {
        return;
    }
    json level = json::parse(file);
    if(!level.contains("nodes")){
        return;
    }
    for(Node& nodes : graph.viewNodeList()){
        graph.deleteNode(nodes.viewEntity());
    }
    graph.clearNodes();
    api.clearInstanceVector(InstanceType::Circle);
    editorState.openedWindows.clear();
    editorState.tempLine = INVALID_ENTITY;
    editorState.holdEntity = INVALID_ENTITY;
    editorState.game = GameState{};
    editorState.game.gameLevelName = filename;
    for(auto& node : level.at("nodes")){
        Entity e = graph.addNode({node.at("Position").at("x").get<float>(), node.at("Position").at("y").get<float>()});
        graph.getNode(e).data.value = node.at("Value").get<int>();
        graph.getNode(e).setType(static_cast<NodeType>(node.at("Type").get<int>()));
        api.getInstance(e).color = graph.getNode(e).data.baseColor;
        graph.getNode(e).data.tags.noDelete = node.at("Tags").at("noDelete").get<bool>();
        graph.getNode(e).data.tags.noMove = node.at("Tags").at("noMove").get<bool>();
        graph.getNode(e).data.tags.noUpdate = node.at("Tags").at("noUpdate").get<bool>();
    }
    for(auto& node : level.at("nodes")){
        if(!node.contains("Links")){
            continue;
        }
        for(auto& link : node.at("Links")){
            graph.connect({node.at("ID").get<uint32_t>(), InstanceType::Circle}, {link.get<uint32_t>(), InstanceType::Circle});
        }
    }
    for(auto& node : level.at("nodes")){
        if(!node.at("Alive").get<uint32_t>()){
            graph.deleteNode({node.at("ID").get<uint32_t>(), InstanceType::Circle});
        }
    }
    forces.nodeRepulsionForce = level.at("NodeRepulsionForce").get<float>();
    forces.nodeRepulsionRadius = level.at("NodeRepulsionRadius").get<float>();
    forces.lineForce = level.at("LineForce").get<float>();
    forces.lineCenter = level.at("LineCenter").get<float>();
    forces.centerAttraction = level.at("CenterAttraction").get<float>();
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
        !forces.centerAttraction ? editorState.config.centerAttraction = false : editorState.config.centerAttraction = true;
        !forces.nodeRepulsionForce ? editorState.config.nodeRepulsion = false : editorState.config.nodeRepulsion = true;
        !forces.lineForce ? editorState.config.lineForces = false : editorState.config.lineForces = true;

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
        loadLevel(api, "Menu");
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
#include "stateMachine.h"
#include "../globals.h"
#include "ThING/api.h"
#include "../auxiliar/window.h"
#include "ThING/types/apiTypes.h"
#include "glm/fwd.hpp"
#include "imgui.h"
#include <string>
#include "../auxiliar/persistence.h"

StateMachine::StateMachine(){
    currentState = StateM::Idle;
    api = nullptr;
    editorState = nullptr;
    returnState = StateM::Idle;
}

void StateMachine::bind(ThING::API& api, EditorState& editorState){
    this->api = &api;
    this->editorState = &editorState;
}

std::string StateMachine::stateToString(){
    switch(currentState){
        case StateM::Idle:             return "Idle";
        case StateM::WaitLeftIdle:     return "WaitLeftIdle";
        case StateM::WaitRightIdle:    return "WaitRightIdle";
        case StateM::DraggingLine:     return "DraggingLine";
        case StateM::DraggingNode:     return "DraggingNode";
        case StateM::PlayingAnimation: return "PlayingAnimation";
        case StateM::DraggingRight:    return "DraggingRight";
        case StateM::GameIdle:         return "GameIdle";
        case StateM::GameWaitLeftIdle: return "GameWaitLeftIdle";
        case StateM::GameWaitRightIdle:return "GameWaitRightIdle";
        case StateM::GameDraggingRight:return "GameDraggingRight";
        case StateM::MenuIdle:         return "MenuIdle";
        case StateM::PanningCamera:    return "PanningCamera";
        case StateM::Menu:             return "Menu";
        default:                       return "UnknownState";
    }
}

void StateMachine::update(){
    if(api == nullptr || editorState == nullptr){
        return;
    }
    switch (currentState) {
        case StateM::Idle:
            idle();
            break;
        case StateM::WaitLeftIdle:
            waitLeftIdle();
            break;
        case StateM::WaitRightIdle:
            waitRightIdle();
            break;
        case StateM::DraggingLine:
            dragginLine();
            break;
        case StateM::DraggingNode:
            draggingNode();
            break;
        case StateM::DraggingRight:
            draggingRight();
            break;
        case StateM::PlayingAnimation:
            playingAnimation();
            break;
        case StateM::GameIdle:
            gameIdle();
            break;
        case StateM::GameWaitLeftIdle:
            gameWaitLeftIdle();
            break;
        case StateM::GameWaitRightIdle:
            gameWaitRightIdle();
            break;
        case StateM::GameDraggingRight:
            gameDraggingRight();
            break;
        case StateM::MenuIdle:
            menuIdle();
            break;
        case StateM::PanningCamera:
            panningCamera();
            break;
        case StateM::Menu:
            menu();
            break;
    }
}

void StateMachine::selectNode(){
    api->playAudio(Style::Audio::SelectNode);
    api->getInstance(editorState->holdEntity).color = editorState->getHoldNode().data.selectedColor;
    glm::vec2 scale = {Style::NodeSize + Style::NodeSelectedPadding, Style::NodeSize + Style::NodeSelectedPadding};
    api->getInstance(editorState->holdEntity).scale = scale;
}

void StateMachine::deselectNode(){
    api->playAudio(Style::Audio::ReleaseNode);
    api->getInstance(editorState->holdEntity).color = editorState->getHoldNode().data.baseColor;
    api->getInstance(editorState->holdEntity).scale = {Style::NodeSize, Style::NodeSize};
}

void StateMachine::idle(){
    if(ImGui::IsKeyPressed(ImGuiKey_Escape, false)){
        returnState = StateM::Idle;
        currentState = StateM::Menu;
        return;
    }
    editorState->holdEntity = INVALID_ENTITY;
    if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)){
        editorState->holdEntity = hitEntity(*api, editorState->windowData);
        if(editorState->holdEntity.valid()){
            selectNode();
        }
        currentState = StateM::WaitLeftIdle;
        return;
    }
    if(ImGui::IsMouseClicked(ImGuiMouseButton_Right)){
        editorState->holdEntity = hitEntity(*api, editorState->windowData);
        if(editorState->holdEntity.valid()){
            selectNode();
        }
        currentState = StateM::WaitRightIdle;
        return;
    }
    if(ImGui::IsMouseClicked(ImGuiMouseButton_Middle)){
        // for(Node& node : editorState->graph->viewNodeList()){
        //     if(api->exists(node.viewEntity())){
        //         currentState = StateM::PlayingAnimation;
        //         break;
        //     }
        // }
        currentState = StateM::GameIdle;
        return;
    }
}

void StateMachine::waitLeftIdle(){
    if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)){
        static int i = 0;
        if(editorState->holdEntity.valid()){
            if(editorState->game.currentNode == editorState->holdEntity){
                deselectNode();
                currentState = StateM::Idle;
                return;
            }
            Node& node = editorState->getHoldNode();
            NodeType newNodeType = static_cast<NodeType>((static_cast<int>(node.data.type) + 1));
            if(newNodeType == NodeType::Count || newNodeType == NodeType::Start){
                node.setType(static_cast<NodeType>(0));
            } else {
                node.setType(newNodeType);
            }
            deselectNode();
            currentState = StateM::Idle;
            return;
        }
        if(!hitEntity(*api, editorState->windowData, Style::NodeSize + Style::OutlineWidth).valid()){
            api->playAudio(Style::Audio::ReleaseNode);
            Entity e = editorState->graph->addNode(mousePosition(editorState->windowData));
            editorState->graph->getNode(e).data.value = 0;
        }
        currentState = StateM::Idle;
        return;
    }
    if(ImGui::IsMouseDragging(ImGuiMouseButton_Left, Style::LockDragging)){
        if(!editorState->holdEntity.valid()){
            returnState = StateM::Idle;
            currentState = StateM::PanningCamera;
            return;
        }
        currentState = StateM::DraggingLine;
    }
}

void StateMachine::waitRightIdle(){
    if(ImGui::IsMouseReleased(ImGuiMouseButton_Right)){
        if(editorState->holdEntity.valid()){
            api->playAudio(Style::Audio::OpenUi);
            api->getInstance(editorState->holdEntity).color = editorState->getHoldNode().data.baseColor;
            api->getInstance(editorState->holdEntity).scale = {Style::NodeSize, Style::NodeSize};
            editorState->openedWindows[editorState->holdEntity] = true;
        }
        currentState = StateM::Idle;
        return;
    }
    if(ImGui::IsMouseDragging(ImGuiMouseButton_Right)){
        if(!editorState->holdEntity.valid()){
            currentState = StateM::DraggingRight;
            return;
        }
        if(editorState->getHoldNode().data.tags.noMove){
            deselectNode();
            editorState->holdEntity = INVALID_ENTITY;
            currentState = StateM::DraggingRight;
            return;
        }
        currentState = StateM::DraggingNode;
    }
}

void StateMachine::dragginLine(){
    if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)){
        if(!editorState->tempLine.valid()){
            currentState = StateM::Idle;
            return;
        }
        api->getInstance(editorState->holdEntity).color = editorState->getHoldNode().data.baseColor;
        api->getInstance(editorState->holdEntity).scale = {Style::NodeSize, Style::NodeSize};
        Entity e = hitEntity(*api, editorState->windowData, api->getLine(editorState->tempLine).point2);
        if(e.valid()){
            if(editorState->holdEntity == e){
                api->playAudio(Style::Audio::ReleaseNode);
            } else if (editorState->graph->connect(editorState->holdEntity, e)){
                api->playAudio(Style::Audio::ConnectNode, 150); // USE MASTEER VOLUME
            } else {
                api->playAudio(Style::Audio::DisconnectNode, 150);
            }
        } else {
            api->playAudio(Style::Audio::ReleaseNode);
        }
        api->deleteInstance(editorState->tempLine);
        editorState->tempLine = INVALID_ENTITY;
        currentState = StateM::Idle;
        return;
    }
    if(!editorState->tempLine.valid()){
        editorState->tempLine = api->addLine(api->getInstance(editorState->holdEntity).position, 
            mousePosition(editorState->windowData), Style::LineWidth);
        api->getLine(editorState->tempLine).color = Style::Color::TempLine;
        api->getLine(editorState->tempLine).outlineColor = Style::Color::Outline;
        api->getLine(editorState->tempLine).outlineSize = Style::OutlineWidth;
        api->getLine(editorState->tempLine).objectID = 1;
    } else {
        api->getLine(editorState->tempLine).point1 = api->getInstance(editorState->holdEntity).position;
        api->getLine(editorState->tempLine).point2 = mousePosition(editorState->windowData);
    }
    api->updateOutlines();
}

void StateMachine::draggingNode(){
    if(ImGui::IsMouseReleased(ImGuiMouseButton_Right)){
        deselectNode();
        currentState = StateM::Idle;
    }
    api->getInstance(editorState->holdEntity).position = mousePosition(editorState->windowData);
    editorState->graph->update();
}

void StateMachine::draggingRight(){
    if(ImGui::IsMouseReleased(ImGuiMouseButton_Right)){
        currentState = StateM::Idle;
    }
}

void StateMachine::playingAnimation(){
    currentState = StateM::Idle;
    return;
    static bool first = true;
    static Node* tempNode = nullptr;
    static std::unordered_map<Node*, int> visited;
    if(first){
        for(Node& node : editorState->graph->viewNodeList()){
            if(api->exists(node.viewEntity())){
                currentState = StateM::PlayingAnimation;
                tempNode = &node;
                visited.clear();
                visited[tempNode] = 0;
                break;
            }
        }
        visited.clear();
        first = false;
    }
    if(ImGui::IsMouseClicked(ImGuiMouseButton_Right)){
        for(LineData& line : api->getLineVector()){
            if (!line.alive){
                continue;
            }
            line.color = Style::Color::Line;
            line.outlineColor = Style::Color::Outline;
            line.objectID = 1;
            api->updateOutlines();
        }
        currentState = StateM::WaitRightIdle;
    }
    if(ImGui::GetFrameCount() % 10){
        return;
    }
    if(!visited.contains(tempNode)){
        visited[tempNode] = 0;
    }
    if(visited[tempNode] < tempNode->links.size()){
        Entity lineE = tempNode->links[visited[tempNode]].viewLine();
        api->getLine(lineE).color *= .8f;
        api->getLine(lineE).color.a = 1;
        api->getLine(lineE).objectID = 2;
        api->updateOutlines();
        Entity e = tempNode->links[visited[tempNode]].viewConnection();
        visited[tempNode]++;
        tempNode = &editorState->graph->getNode(e);
    } else if (tempNode->links.size() == 0){
        for(LineData& line : api->getLineVector()){
            if (!line.alive){
                continue;
            }
            line.color = Style::Color::Line;
            line.outlineColor = Style::Color::Outline;
            line.objectID = 1;
            api->updateOutlines();
        }
        first = true;
        currentState = StateM::Idle;
    } else {
        visited[tempNode] = 0;
        Entity lineE = tempNode->links[visited[tempNode]].viewLine();
        api->getLine(lineE).color *= .8f;
        api->getLine(lineE).color.a = 1;
        api->getLine(lineE).objectID = 2;
        api->updateOutlines();
        Entity e = tempNode->links[visited[tempNode]].viewConnection();
        visited[tempNode]++;
        tempNode = &editorState->graph->getNode(e);
    }
}

void StateMachine::gameIdle(){
    if(ImGui::IsKeyPressed(ImGuiKey_Escape, false)){
        returnState = StateM::GameIdle;
        currentState = StateM::Menu;
        return;
    }
    editorState->holdEntity = INVALID_ENTITY;
    if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)){
        editorState->holdEntity = hitEntity(*api, editorState->windowData, 10.f);
        if(!(editorState->holdEntity.valid() && editorState->game.currentNode.valid())){
            currentState = StateM::GameWaitLeftIdle;
            return;
        }
        for(Link& link : editorState->getCurrentNode().links){
            if(link.viewConnection() != editorState->holdEntity || !link.active){
                continue;
            }
            // Here you can detect the type node to play different sounds
            api->playAudio(Style::Audio::SelectNode);
            api->getInstance(editorState->game.currentNode).color = editorState->getCurrentNode().data.baseColor;
            api->getInstance(editorState->game.currentNode).scale = {Style::NodeSize, Style::NodeSize};
            editorState->game.currentNode = editorState->holdEntity;
            currentState = StateM::GameWaitLeftIdle;
            return;
        }
        api->playAudio(Style::Audio::ReleaseNode);
        
        currentState = StateM::GameWaitLeftIdle;
        return;
    }
    if(ImGui::IsMouseClicked(ImGuiMouseButton_Right)){
        editorState->holdEntity = hitEntity(*api, editorState->windowData);
        currentState = StateM::GameWaitRightIdle;
        return;
    }
    if(ImGui::IsMouseClicked(ImGuiMouseButton_Middle)){
        if(!editorState->game.currentNode.valid()){
            currentState = StateM::Idle;
            return;
        }
        editorState->game.points = 0;
        api->getInstance(editorState->game.currentNode).color = editorState->getCurrentNode().data.baseColor;
        api->getInstance(editorState->game.currentNode).outlineColor = Style::Color::Outline;
        api->getInstance(editorState->game.currentNode).objectID = 1;
        api->updateOutlines(); // This can be optimize as updateOutline is only needed when a new objectID is used
        editorState->game.currentNode = INVALID_ENTITY;
        currentState = StateM::Idle;
        return;
    }
}

void StateMachine::gameWaitLeftIdle(){
    if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)){
        // Do Something?
        currentState = StateM::GameIdle;
        return;
    }
    if(ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
        returnState = StateM::GameIdle;
        currentState = StateM::PanningCamera;
        return;
    }
}

void StateMachine::gameWaitRightIdle(){
    if(ImGui::IsMouseReleased(ImGuiMouseButton_Right)){
        // Do something
        currentState = StateM::GameIdle;
        return;
    }
    if(ImGui::IsMouseDragging(ImGuiMouseButton_Right)){
        // Do Something
        currentState = StateM::GameDraggingRight;
        return;
    }
}

void StateMachine::gameDraggingRight(){
    if(ImGui::IsMouseReleased(ImGuiMouseButton_Right)){
        // Do Something
        currentState = StateM::GameIdle;
        return;
    }
}

void StateMachine::menuIdle(){
    if(ImGui::IsKeyPressed(ImGuiKey_Escape, false)){
        returnState = StateM::MenuIdle;
        currentState = StateM::Menu;
        return;
    }
    editorState->holdEntity = INVALID_ENTITY;
    if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)){
        editorState->holdEntity = hitEntity(*api, editorState->windowData, 10.f);
        if(!(editorState->holdEntity.valid() && editorState->game.currentNode.valid())){
            returnState = StateM::MenuIdle;
            currentState = StateM::PanningCamera;
            return;
        }
        if(editorState->getHoldNode().data.type == NodeType::Bad){
            api->playAudio(Style::Audio::ReleaseNode);
            return;
        }
        for(Link& link : editorState->getCurrentNode().links){
            if(!(link.viewConnection() == editorState->holdEntity)){
                continue;
            }
            api->playAudio(Style::Audio::SelectNode);
            api->getInstance(editorState->game.currentNode).color = editorState->getCurrentNode().data.baseColor;
            api->getInstance(editorState->game.currentNode).scale = {Style::NodeSize, Style::NodeSize};
            editorState->game.currentNode = editorState->holdEntity;
            currentState = StateM::MenuIdle;
            return;
        }
        if(editorState->getHoldNode() == editorState->getCurrentNode()){
            editorState->game.level = editorState->game.currentNode.index;
            if(loadLevel(*api, std::to_string(editorState->game.currentNode.index))){
                currentState = StateM::GameIdle;
            }
        }
        return;
    }
}

void StateMachine::panningCamera(){
    if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)){
        currentState = returnState;
        return;
    }
    {
        ImVec2 delta = ImGui::GetIO().MouseDelta;
        editorState->windowData.offset.x -= delta.x / editorState->windowData.zoom;
        editorState->windowData.offset.y -= delta.y / editorState->windowData.zoom;
        api->setOffset(editorState->windowData.offset);
    }
}

void StateMachine::menu(){
    // This function is here to clarify, menu logic is in uiCallback
}
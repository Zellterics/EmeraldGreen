#include "stateMachine.h"
#include "../globals.h"
#include "ThING/api.h"
#include "../auxiliar/window.h"
#include "ThING/types/apiTypes.h"
#include "imgui.h"

StateMachine::StateMachine(){
    currentState = StateM::Idle;
    api = nullptr;
    editorState = nullptr;
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
        case StateM::PanningCamera:    return "PanningCamera";
        case StateM::PlayingAnimation: return "Playing Animation";
        case StateM::DraggingRight:    return "Dragging Right";
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
        case StateM::PanningCamera:
            panningCamera();
            break;
        case StateM::DraggingRight:
            draggingRight();
            break;
        case StateM::PlayingAnimation:
            playingAnimation();
            break;
    }
}

void StateMachine::selectNode(){
    api->playAudio(Style::Audio::SelectNode);
    api->getInstance(editorState->holdEntity).color = Style::Color::NodeSelected;
    glm::vec2 scale = {Style::NodeSize + Style::NodeSelectedPadding, Style::NodeSize + Style::NodeSelectedPadding};
    api->getInstance(editorState->holdEntity).scale = scale;
}

void StateMachine::deselectNode(){
    api->playAudio(Style::Audio::ReleaseNode);
    api->getInstance(editorState->holdEntity).color = Style::Color::Node;
    api->getInstance(editorState->holdEntity).scale = {Style::NodeSize, Style::NodeSize};
}

void StateMachine::idle(){
    editorState->holdEntity = INVALID_ENTITY;
    if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)){
        editorState->holdEntity = hitEntity(*api, editorState->windowData);
        if(editorState->holdEntity != INVALID_ENTITY){
            selectNode();
        }
        currentState = StateM::WaitLeftIdle;
        return;
    }
    if(ImGui::IsMouseClicked(ImGuiMouseButton_Right)){
        editorState->holdEntity = hitEntity(*api, editorState->windowData);
        if(editorState->holdEntity != INVALID_ENTITY){
            selectNode();
        }
        currentState = StateM::WaitRightIdle;
        return;
    }
    if(ImGui::IsMouseClicked(ImGuiMouseButton_Middle)){
        for(Node& node : editorState->graph->viewNodeList()){
            if(api->exists(node.viewEntity())){
                currentState = StateM::PlayingAnimation;
                break;
            }
        }
    }
}

void StateMachine::waitLeftIdle(){
    if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)){
        static int i = 0;
        if(editorState->holdEntity != INVALID_ENTITY){
            editorState->game.points += editorState->graph->getNode(editorState->holdEntity.index).data.value;
            deselectNode();
            currentState = StateM::Idle;
            return;
        }
        if(hitEntity(*api, editorState->windowData, Style::NodeSize + Style::OutlineWidth) == INVALID_ENTITY){
            api->playAudio(Style::Audio::ReleaseNode);
            Entity e = editorState->graph->addNode(mousePosition(editorState->windowData));
            editorState->graph->getNode(e.index).data.value = ++i;
        }
        currentState = StateM::Idle;
        return;
    }
    if(ImGui::IsMouseDragging(ImGuiMouseButton_Left, Style::LockDragging)){
        if(editorState->holdEntity == INVALID_ENTITY){
            currentState = StateM::PanningCamera;
            return;
        }
        currentState = StateM::DraggingLine;
    }
}

void StateMachine::waitRightIdle(){
    if(ImGui::IsMouseReleased(ImGuiMouseButton_Right)){
        if(editorState->holdEntity != INVALID_ENTITY){
            api->playAudio(Style::Audio::OpenUi);
            api->getInstance(editorState->holdEntity).color = Style::Color::Node;
            api->getInstance(editorState->holdEntity).scale = {Style::NodeSize, Style::NodeSize};
            editorState->openedWindows[editorState->holdEntity] = true;
        }
        currentState = StateM::Idle;
        return;
    }
    if(ImGui::IsMouseDragging(ImGuiMouseButton_Right)){
        if(editorState->holdEntity == INVALID_ENTITY){
            currentState = StateM::DraggingRight;
            return;
        }
        if(editorState->graph->getNode(editorState->holdEntity.index).data.tags.noMove){
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
        if(editorState->tempLine == INVALID_ENTITY){
            currentState = StateM::Idle;
            return;
        }
        api->getInstance(editorState->holdEntity).color = Style::Color::Node;
        api->getInstance(editorState->holdEntity).scale = {Style::NodeSize, Style::NodeSize};
        Entity e = hitEntity(*api, editorState->windowData, api->getLine(editorState->tempLine).point2);
        if(e != INVALID_ENTITY){
            if(editorState->holdEntity == e){
                api->playAudio(Style::Audio::ReleaseNode);
            } else if (editorState->graph->connect(editorState->holdEntity, e)){
                api->playAudio(Style::Audio::ConnectNode, 150);
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
    if(editorState->tempLine == INVALID_ENTITY){
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

void StateMachine::panningCamera(){
    if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)){
        currentState = StateM::Idle;
        return;
    }
    {
        ImVec2 delta = ImGui::GetIO().MouseDelta;
        editorState->windowData.offset.x -= delta.x / editorState->windowData.zoom;
        editorState->windowData.offset.y -= delta.y / editorState->windowData.zoom;
        api->setOffset(editorState->windowData.offset);
    }
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
        tempNode = &editorState->graph->getNode(e.index);
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
        tempNode = &editorState->graph->getNode(e.index);
    }
}
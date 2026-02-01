#include "stateMachine.h"
#include "../globals.h"
#include "ThING/api.h"
#include "../auxiliar/window.h"

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
        case StateM::PlayingAnimation:
            playingAnimation();
            break;
    }
}

void StateMachine::idle(){
    editorState->holdEntity = INVALID_ENTITY;
    if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)){
        editorState->holdEntity = hitEntity(*api, editorState->windowData);
        currentState = StateM::WaitLeftIdle;
    }
    if(ImGui::IsMouseClicked(ImGuiMouseButton_Right)){
        editorState->holdEntity = hitEntity(*api, editorState->windowData);
        currentState = StateM::WaitRightIdle;
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
            currentState = StateM::DraggingLine;
            return;
        }
        if(hitEntity(*api, editorState->windowData, Style::NodeSize + Style::OutlineWidth) == INVALID_ENTITY){
            Entity e = editorState->graph->addNode(mousePosition(editorState->windowData));
            editorState->graph->getNode(e.index).data.value = i++;
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
            editorState->openedWindows[editorState->holdEntity] = true;
        }
        currentState = StateM::Idle;
        return;
    }
    if(ImGui::IsMouseDragging(ImGuiMouseButton_Right)){
        currentState = StateM::DraggingNode;
    }
}

void StateMachine::dragginLine(){
    if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)){
        if(editorState->tempLine != INVALID_ENTITY){
            api->deleteInstance(editorState->tempLine);
            editorState->tempLine = INVALID_ENTITY;
        }
        if(api->exists(editorState->holdEntity)){
            editorState->graph->connect(editorState->holdEntity, hitEntity(*api, editorState->windowData));
        }
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
        currentState = StateM::Idle;
    }
    if(editorState->holdEntity != INVALID_ENTITY){
        api->getInstance(editorState->holdEntity).position = mousePosition(editorState->windowData);
        editorState->graph->update();
    }
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

void StateMachine::playingAnimation(){
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
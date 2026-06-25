#include "update.h"
#include "../globals.h"
#include "../stateMachine/stateMachine.h"
#include "../auxiliar/persistence.h"

void updateCallback(ThING::API& api, FPSCounter& fps){
    static Graph& graph = *editorState.graph; 

    api.getWindowSize(&editorState.windowData.size.width, &editorState.windowData.size.height);

    if(!api.exists(editorState.game.currentNode)){
        editorState.game.currentNode = INVALID_ENTITY;
    }

    selectCurrentNode(api);

    updateGraph();

    handleFinishTimer(&api, fps.getDeltaTime());    

    ImGuiIO& io = ImGui::GetIO();
    if(!io.WantCaptureMouse){
        stateMachine.update();
    }

    navigationLogic(api);
}

void handleFinishTimer(ThING::API* api, float deltaTime){
    static float secondCounter = 0;
    if(editorState.game.currentNode.valid()){
        if(editorState.graph->getNode(editorState.game.currentNode).data.type == NodeType::Goal && editorState.game.winTimer < 0 && editorState.game.loseTimer < 0){
            Node& winNode = editorState.graph->getNode(editorState.game.currentNode);
            if(winNode.data.value >= 0){
                api->playAudio(Style::Audio::Win);
                secondCounter = 0;
                editorState.game.winTimer = 1;
            } else {
                api->playAudio(Style::Audio::Lose);
                secondCounter = 0;
                editorState.game.loseTimer = 1;
            }
        }
    }
    if(editorState.game.winTimer > 0){
        secondCounter += deltaTime;
        if(secondCounter >= 1){
            secondCounter = 0;
            editorState.game.winTimer--;
            if(editorState.game.winTimer <= 0){
                stateMachine.getState() = StateM::MenuIdle;
                editorState.game.currentNode = INVALID_ENTITY;
                editorState.game.winTimer = -1;
                loadLevel(*api, "Menu");
            }
        }
        return;
    }
    if(editorState.game.loseTimer > 0){
        secondCounter += deltaTime;
        if(secondCounter >= 1){
            secondCounter = 0;
            editorState.game.loseTimer--;
            if(editorState.game.loseTimer <= 0){
                loadLevel(*api, editorState.game.gameLevelName);
                stateMachine.getState() = StateM::GameIdle;
                editorState.game.points = 0;
                editorState.game.currentNode = INVALID_ENTITY;
                editorState.game.loseTimer = -1;
            }
        }
        return;
    }
}

void selectCurrentNode(ThING::API& api){
    static Graph& graph = *editorState.graph;
    if(!editorState.game.currentNode.valid() && graph.viewNodeList().size() > 0){
        for(Node& node : graph.viewNodeList()){
            if(api.getInstance(node.viewEntity()).alive){
                if(graph.getNode(node.viewEntity()).data.type == NodeType::Start){
                    editorState.game.currentNode = node.viewEntity();
                    break;
                }
            }
        }
        if(!editorState.game.currentNode.valid()){
            for(Node& node : graph.viewNodeList()){
                if(api.getInstance(node.viewEntity()).alive){
                    editorState.game.currentNode = node.viewEntity();
                    graph.getNode(editorState.game.currentNode).setType(NodeType::Start);
                    break;
                }
            }
        }
    }
}

void updateGraph(){
    static Graph& graph = *editorState.graph; 
    if(stateMachine.getState() != StateM::Menu){
        graph.applyForces(forces, editorState.config.forceFlags);
        graph.update();
    }

    for(Node& node : graph.viewNodeList()){
        if(node.data.type == NodeType::Goal){
            node.data.value = editorState.game.points;
        }
    }
}

void navigationLogic(ThING::API& api){
    static Graph& graph = *editorState.graph; 
    static Entity lastNode = editorState.game.currentNode;
    if(editorState.game.currentNode.valid()){
        if(!lastNode.valid()){
            lastNode = editorState.game.currentNode;
        }
        if(lastNode != editorState.game.currentNode){
            lastNode = editorState.game.currentNode;
            switch (graph.getNode(lastNode).data.type) {
                case NodeType::Bad:
                    if(graph.getNode(lastNode).data.value == 0){
                        editorState.game.points--;
                    } else {
                        editorState.game.points -= graph.getNode(lastNode).data.value;
                    }
                    break;
                case NodeType::None:
                    break;
                case NodeType::Good:
                    if(graph.getNode(lastNode).data.value == 0){
                        editorState.game.points++;
                    } else {
                        editorState.game.points += graph.getNode(lastNode).data.value;
                    }
                    graph.getNode(lastNode).setType(NodeType::None);
                    api.getInstance(lastNode).color = Style::Color::NodeSelected;
                    break;
                case NodeType::Goal:
                    //editorState.game.points += 100;
                    break;
                case NodeType::Start:
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
#include "../../external/json.hpp"
#include "ThING/types/enums.h"
#include "../globals.h"
#include "persistence.h"
#include "../stateMachine/stateMachine.h"
#include "style.h"

using json = nlohmann::json;
void saveLevel(ThING::API& api, std::string filename){
    zlog.debug("Saving {} file level", filename);
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
            json jLink;
            jLink["targetID"] = link.viewConnection().index;
            jLink["active"] = link.active;
            jNode["Links"].push_back(jLink);
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

bool loadLevel(ThING::API& api, std::string filename){
    Graph& graph = *editorState.graph;
    std::ifstream file(filename + ".json");
    if (!file.is_open()) {
        zlog.warn("level filename [{}] not found", filename);
        return false;
    }
    json level = json::parse(file);
    zlog.debug("Loading {} file level", filename);
    if(!level.contains("nodes")){
        zlog.warn("level named [{}] is empty", filename);
        return false;
    }
    for(Node& nodes : graph.viewNodeList()){
        graph.deleteNode(nodes.viewEntity());
    }
    graph.clearNodes();
    api.clearInstanceVector(InstanceType::Circle);
    editorState.openedWindows.clear();
    editorState.tempLine = INVALID_ENTITY;
    editorState.holdEntity = INVALID_ENTITY;
    int nodeLevelID = editorState.game.level;
    editorState.game = GameState{};
    editorState.game.gameLevelName = filename;
    editorState.game.level = nodeLevelID;
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
        Entity e { node.at("ID").get<uint32_t>(), InstanceType::Circle};
        for(auto& link : node.at("Links")){
            graph.connect({node.at("ID").get<uint32_t>(), InstanceType::Circle}, {link.at("targetID").get<uint32_t>(), InstanceType::Circle});
            graph.getNode(e).links.back().active = link.at("active").get<bool>();
            if(!graph.getNode(e).links.back().active){
                api.getInstance(graph.getNode(e).links.back().viewLine()).color = Style::Color::InnactiveLine;
            }
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
    return true;
}

void loadMenu(ThING::API& api){
    Graph& graph = *editorState.graph;
    loadLevel(api, "Menu");
    zlog.debug("Node Level ID = {}", editorState.game.level);
    if(editorState.game.level == -1){
        zlog.log("loaded Menu without winned level, level = {}", Zlog::location(), editorState.game.level);
        return;
    }
    int nodeLevelID = editorState.game.level;
    if(!graph.getNode(nodeLevelID).viewEntity().valid()){
        zlog.warn("In {} -> Invalid Level Provided: Level = {}", Zlog::location(), editorState.game.level);
        return;
    }
    for (Link& link : graph.getNode(nodeLevelID).links){
        if(graph.getNode(link.viewConnection()).data.type == NodeType::Bad){
            graph.getNode(link.viewConnection()).setType(NodeType::Good);
            api.getInstance(link.viewConnection()).color = graph.getNode(link.viewConnection()).data.baseColor;
        }
        graph.getNode(nodeLevelID).setType(NodeType::None);
    }
    
}

void restartLevel(ThING::API& api){
    loadLevel(api, editorState.game.gameLevelName);
    stateMachine.getState() = StateM::GameIdle;
    editorState.game.points = 0;
    editorState.game.currentNode = INVALID_ENTITY;
    editorState.game.loseTimer = -1;
}
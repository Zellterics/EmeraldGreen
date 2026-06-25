#include "../../external/json.hpp"
#include "ThING/types/enums.h"
#include "../globals.h"
#include "persistence.h"

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
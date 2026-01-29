#include "graph.h"
#include "ThING/api.h"
#include "ThING/types/apiTypes.h"
#include "ThING/types/enums.h"
#include "ThING/types/renderData.h"
#include "link.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include "../auxiliar/style.h"

Graph::Graph(ThING::API& api) : api(api){

}

Entity Graph::addNode(glm::vec2 pos){
    Entity e;
    e = api.addCircle(pos, Color::NodeSize, Color::Node);
    api.getInstance(e).outlineSize = Color::OutlineWidth;
    api.getInstance(e).outlineColor = Color::Outline;
    api.getInstance(e).objectID = 1;
    api.getInstance(e).drawIndex = 20;
    api.updateOutlines();
    if(e.index < nodes.size()){
        nodes[e.index].links.clear();
        return e;
    }
    nodes.emplace_back(e);
    return e;
}

void Graph::deleteNode(Entity e){
    api.deleteInstance(e);
    for(Node& node : nodes){
        node.links.erase(
            std::remove_if(node.links.begin(), node.links.end(), 
                [&](const Link& l){return l.viewConnection() == e;}),
            node.links.end());
    }
}

void Graph::connect(Entity from, Entity to){
    if(api.exists(from) && api.exists(to)){
        for(Link& link : nodes[from.index].links){
            if(link.viewConnection() == to){
                return;
            }
        }
        Entity line = api.addLine(api.getInstance(from).position, api.getInstance(to).position, Color::LineWidth);
        nodes[from.index].connect(to, line);
        api.getLine(line).color = Color::Line;
        api.getLine(line).outlineColor = Color::Outline;
        api.getLine(line).outlineSize = Color::OutlineWidth;
        api.getLine(line).objectID = 1;
    }
    
}

void Graph::update(){
    std::span<LineData> lines = api.getLineVector();
    for(uint32_t i = 0; i < lines.size(); i++){
        api.deleteInstance({i, InstanceType::Line});
    }
    size_t i = 0;
    for(Node& node : nodes){
        if(!api.exists(node.viewEntity())){
            continue;
        }
        
        for(Link& link : node.links){
            Entity e = api.addLine(api.getInstance(node.viewEntity()).position, api.getInstance(link.viewConnection()).position, Color::LineWidth);
            api.getLine(e).color = Color::Line;
            api.getLine(e).outlineColor = Color::Outline;
            api.getLine(e).outlineSize = Color::OutlineWidth;
            api.getLine(e).objectID = 1;
        }
    }
}
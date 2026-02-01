#include "graph.h"
#include "ThING/api.h"
#include "ThING/types/apiTypes.h"
#include "ThING/types/renderData.h"
#include "link.h"
#include <algorithm>
#include <cstddef>
#include "../auxiliar/style.h"

Graph::Graph(ThING::API& api) : api(api), nullNode(INVALID_ENTITY){
    
}

Entity Graph::addNode(glm::vec2 pos){
    Entity e;
    e = api.addCircle(pos, Style::NodeSize, Style::Color::Node);
    api.getInstance(e).outlineSize = Style::OutlineWidth;
    api.getInstance(e).outlineColor = Style::Color::Outline;
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
    for(Link& link : getNode(e.index).links){
        api.deleteInstance(link.viewLine());
    }
    api.deleteInstance(e);
    for(Node& node : nodes){
        node.links.erase(
            std::remove_if(node.links.begin(), node.links.end(), 
                [&](const Link& l){
                        if( l.viewConnection() == e){
                            api.deleteInstance(l.viewLine());
                            return true;
                        }
                        return false;
                    }),
            node.links.end());
    }
}

void Graph::connect(Entity from, Entity to){
    if(!(api.exists(from) && api.exists(to))){
        return;
    }
    if(from == to){
        return;
    }
    Node& node = nodes[from.index];
    for(size_t i = 0; i < node.links.size(); i++){
        if(node.links[i].viewConnection() == to){
            api.deleteInstance(node.links[i].viewLine());
            node.links.erase(
                std::remove_if(node.links.begin(), node.links.end(), 
                    [&](const Link& l) {return l.viewConnection() == to;}),
                node.links.end()
            );
            update();
            return;
        }
    }
    Entity line = api.addLine(api.getInstance(from).position, api.getInstance(to).position, Style::LineWidth);
    node.connect(to, line);
    api.getLine(line).color = Style::Color::Line;
    api.getLine(line).outlineColor = Style::Color::Outline;
    api.getLine(line).outlineSize = Style::OutlineWidth;
    api.getLine(line).objectID = 1;
}

void Graph::update(){
    size_t i = 0;
    for(Node& node : nodes){
        if(!api.exists(node.viewEntity())){
            continue;
        }
        
        for(Link& link : node.links){
            api.getLine(link.viewLine()).point1 = api.getInstance(node.viewEntity()).position;
            api.getLine(link.viewLine()).point2 = api.getInstance(link.viewConnection()).position;
        }
    }
}
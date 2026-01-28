#include "graph.h"
#include "ThING/api.h"
#include "ThING/types/apiTypes.h"
#include "ThING/types/renderData.h"
#include "link.h"
#include <algorithm>
#include <cstddef>
#include <span>

Graph::Graph(ThING::API& api) : api(api){

}

Entity Graph::addNode(glm::vec2 pos){
    Entity e;
    e = api.addCircle(pos, 10, {1,.4,.7,1});
    api.getInstance(e).outlineSize = 2;
    api.getInstance(e).outlineColor = {0, 0, 0, 1};
    api.getInstance(e).objectID = 1;
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
        Entity line = api.addLine(api.getInstance(from).position, api.getInstance(to).position, 2);
        nodes[from.index].connect(to, line);
    }
    
}

void Graph::update(){
    std::span<LineData> lines = api.getLineVector();
    size_t i = 0;
    for(Node& node : nodes){
        for(Link& link : node.links){
            api.getLine(link.viewLine()).point1 = api.getInstance(node.viewEntity()).position;
            api.getLine(link.viewLine()).point2 = api.getInstance(link.viewConnection()).position;
        }
    }
}
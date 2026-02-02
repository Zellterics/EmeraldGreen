#include "graph.h"
#include "ThING/api.h"
#include "ThING/types/apiTypes.h"
#include "ThING/types/renderData.h"
#include "link.h"
#include <algorithm>
#include <cstddef>
#include "../auxiliar/style.h"
#include "../globals.h"

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
        nodes[e.index].data = NodeData{};
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

bool Graph::connect(Entity from, Entity to){
    if(!(api.exists(from) && api.exists(to))){
        return false;
    }
    if(from == to){
        return false;
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
            return false;
        }
    }
    Entity line = api.addLine(api.getInstance(from).position, api.getInstance(to).position, Style::LineWidth);
    node.connect(to, line);
    api.getLine(line).color = Style::Color::Line;
    api.getLine(line).outlineColor = Style::Color::Outline;
    api.getLine(line).outlineSize = Style::OutlineWidth;
    api.getLine(line).objectID = 1;
    return true;
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

void Graph::applyLineForces(Forces forces){
    for(Node& node : nodes){
        glm::vec2& nodePosition = api.getInstance(node.viewEntity()).position;
        for(Link& link : node.links){
            if(!api.exists(link.viewLine())){
                continue;
            }
            glm::vec2& pos1 = api.getLine(link.viewLine()).point1;
            glm::vec2& pos2 = api.getLine(link.viewLine()).point2;
            float distance = length(pos2 - pos1);
            glm::vec2 direction = normalize(pos2 - pos1);
            glm::vec2 force = direction * (distance - forces.lineCenter) * forces.lineForce;
            if(!node.data.tags.noUpdate){
                nodePosition += force;
            }
            if(!editorState.graph->getNode(link.viewConnection().index).data.tags.noUpdate){
                api.getInstance(link.viewConnection()).position -= force;
            }
        }
    }
}

void Graph::applyNodeRepulsion(Forces forces){
    std::unordered_map<int64_t, std::vector<Entity>> hashGrid;
    hashGrid.reserve(nodes.size());
    for(Node& node : nodes){
        glm::vec2& nodePosition = api.getInstance(node.viewEntity()).position;
        hashGrid[(int64_t(floor(nodePosition.x / forces.nodeRepulsionRadius)) << 32) | (uint32_t(floor(nodePosition.y / forces.nodeRepulsionRadius)))].push_back(node.viewEntity());
    }
    for(Node& node : nodes){
        if(!api.exists(node.viewEntity())){
            continue;
        }
        if(node.data.tags.noUpdate){
            continue;
        }
        glm::vec2& nodePosition = api.getInstance(node.viewEntity()).position;
        int64_t nodeKey = (int64_t(floor(nodePosition.x / forces.nodeRepulsionRadius)) << 32) | (uint32_t(floor(nodePosition.y / forces.nodeRepulsionRadius)));
        int xKey = int(floor(nodePosition.x / forces.nodeRepulsionRadius));
        int yKey = int(floor(nodePosition.y / forces.nodeRepulsionRadius));

        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                int nx = xKey + i;
                int ny = yKey + j;

                int64_t key = (int64_t(nx) << 32) | (uint32_t(ny));

                auto it = hashGrid.find(key);
                if (it == hashGrid.end())
                    continue;

                for (Entity& other : it->second) {
                    if (other == node.viewEntity() || !api.exists(other))
                        continue;
                    glm::vec2 otherPos = api.getInstance(other).position;
                    glm::vec2 delta = nodePosition - otherPos;
                    float dist2 = dot(delta, delta);

                    float minDist = forces.nodeRepulsionRadius;
                    float minDist2 = minDist * minDist;

                    if (dist2 > 0.0001f && dist2 < minDist2) {

                        float dist = sqrt(dist2);
                        glm::vec2 dir = delta / dist;

                        float t = (minDist - dist) / minDist;
                        t = std::clamp(t, 0.f, 1.f);

                        nodePosition += dir * t * forces.nodeRepulsionForce;
                    }
                }
            }
        }
    }
}

void Graph::applyCenterAttraction(Forces forces){
    for(Node& node : nodes){
        if(node.data.tags.noUpdate){
            continue;
        }
        api.getInstance(node.viewEntity()).position *= 1 - forces.centerAttraction;
    }
}
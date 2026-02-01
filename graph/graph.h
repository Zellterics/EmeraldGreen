#pragma once

#include "ThING/api.h"
#include "ThING/types/apiTypes.h"
#include "glm/fwd.hpp"
#include "node.h"
#include <span>

struct Forces;

class Graph{
public:
    Graph(ThING::API& api);
    Entity addNode(glm::vec2 pos);
    void deleteNode(Entity e);
    void connect(Entity from, Entity to);
    void update();
    Node& last(){return nodes.empty() ? nullNode : nodes.back();}
    Node& getNode(int i) {return i < nodes.size() ? nodes[i] : nullNode;};
    std::span<Node> viewNodeList() {return nodes;}

    void applyLineForces(Forces forces);
    void applyNodeRepulsion(Forces forces);
    void applyCenterAttraction(Forces forces);
private:
    Node nullNode;
    std::vector<Node> nodes;
    ThING::API& api;
};
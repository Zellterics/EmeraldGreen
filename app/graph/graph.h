#pragma once

#include "ThING/api.h"
#include "ThING/types/apiTypes.h"
#include "glm/fwd.hpp"
#include "node.h"
#include <cstdint>
#include <span>

struct Forces;

enum ForceFlags_ : uint8_t{
    ForceFlags_None = 0,
    ForceFlags_SkipLines = 1 << 0,
    ForceFlags_SkipNodeRepulsion = 1 << 1,
    ForceFlags_SkipCenterAttraction = 1 << 2,
};

class Graph{
public:
    Graph(ThING::API& api);
    Entity addNode(glm::vec2 pos);
    void deleteNode(Entity e);
    bool connect(Entity from, Entity to);
    void update();
    Node& last(){return nodes.empty() ? nullNode : nodes.back();}
    Node& getNode(int i) {return i < nodes.size() ? nodes[i] : nullNode;};
    Node& getNode(Entity e) {return e.index < nodes.size() ? nodes[e.index] : nullNode;}
    std::span<Node> viewNodeList() {return nodes;}

    void clearNodes(){nodes.clear();}

    void applyForces(Forces forces, const uint8_t forceFlags);

    void applyLineForces(Forces forces);
    void applyNodeRepulsion(Forces forces);
    void applyCenterAttraction(Forces forces);
private:
    Node nullNode;
    std::vector<Node> nodes;
    ThING::API& api;
};
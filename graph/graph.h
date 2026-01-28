#pragma once

#include "ThING/api.h"
#include "ThING/types/apiTypes.h"
#include "glm/fwd.hpp"
#include "node.h"

class Graph{
public:
    Graph(ThING::API& api);
    Entity addNode(glm::vec2 pos);
    void deleteNode(Entity e);
    void connect(Entity from, Entity to);
    void update();
    Entity first(){return nodes[0].viewEntity();}
private:
    std::vector<Node> nodes;
    ThING::API& api;
};
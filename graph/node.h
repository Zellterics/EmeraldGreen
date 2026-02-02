#pragma once

#include "ThING/types/apiTypes.h"
#include "link.h"
#include <vector>

struct Tags {
    bool noUpdate = false;
    bool noMove = false;
    bool noDelete = false;
};

struct NodeData{
    int value = 0;
    int delay = 0;
    Tags tags;
};

class Node{
public:
    std::vector<Link> links;

    Node(Entity circle){this->circle = circle;}
    void connect(Entity other, Entity line){links.emplace_back(other, line);}
    Entity viewEntity() const {return circle;}
    NodeData data;
private:
    Entity circle;
};
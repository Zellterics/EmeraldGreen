#pragma once

#include "ThING/types/apiTypes.h"
#include "link.h"
#include <vector>

struct NodeData{
    int value;
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
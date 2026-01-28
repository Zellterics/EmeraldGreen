#pragma once

#include "ThING/types/apiTypes.h"
#include "link.h"
#include <vector>

class Node{
public:
    std::vector<Link> links;

    Node(Entity circle){this->circle = circle;}
    void connect(Entity other, Entity line){links.emplace_back(other, line);}
    Entity viewEntity() const {return circle;}
private:
    Entity circle;
};
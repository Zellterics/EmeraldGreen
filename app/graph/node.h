#pragma once

#include "ThING/types/apiTypes.h"
#include "link.h"
#include <vector>
#include "../auxiliar/style.h"

enum class NodeType{
    None,
    Bad,
    Good,
    Goal,
    Switch,
    Start,
    Count
};

struct Tags {
    bool noUpdate = false;
    bool noMove = false;
    bool noDelete = false;
};

struct NodeData{
    int value = 0;
    int delay = 0;
    Tags tags;
    NodeType type = NodeType::None;
    glm::vec4 baseColor = Style::Color::Node;
    glm::vec4 selectedColor = Style::Color::NodeSelected;
};

class Node{
public:
    std::vector<Link> links;

    Node(Entity circle){
        this->circle = circle;
        data.baseColor = Style::Color::Node;
        data.selectedColor = Style::Color::NodeSelected;
    }
    void setType(NodeType type){
        switch (type) {
            case NodeType::None:
                data.baseColor = Style::Color::Node;
                data.selectedColor = Style::Color::NodeSelected;
                data.type = NodeType::None;
                break;
            case NodeType::Bad:
                data.baseColor = Style::Color::NodeBad;
                data.selectedColor = Style::Color::NodeBadSelected;
                data.type = NodeType::Bad;
                break;
            case NodeType::Good:
                data.baseColor = Style::Color::NodeGood;
                data.selectedColor = Style::Color::NodeGoodSelected;
                data.type = NodeType::Good;
                break;
            case NodeType::Goal:
                data.baseColor = Style::Color::NodeGoal;
                data.selectedColor = Style::Color::NodeGoalSelected;
                data.type = NodeType::Goal;
                break;
            case NodeType::Switch:
                data.baseColor = Style::Color::NodeSwitch;
                data.selectedColor = Style::Color::NodeSwitchSelected;
                data.type = NodeType::Switch;
                break;
            case NodeType::Start:
                data.baseColor = Style::Color::Node;
                data.selectedColor = Style::Color::NodeSelected;
                data.type = NodeType::Start;
                break;
            case NodeType::Count:
                data.baseColor = Style::Color::Node;
                data.selectedColor = Style::Color::NodeSelected;
                data.type = NodeType::None;
                break;
        }
    }
    void connect(Entity other, Entity line){links.emplace_back(other, line);}
    Entity viewEntity() const {return circle;}
    NodeData data;

    bool operator==(const Node& other) const {
        if(other.viewEntity() == this->viewEntity()){
            return true;
        }
        return false;
    }

private:
    Entity circle;
};
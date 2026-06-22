#pragma once
#include "ThING/types/apiTypes.h"
#include "auxiliar/style.h"
#include "graph/graph.h"
#include "auxiliar/window.h"
#include "imgui.h"

struct GameState{
    int points = 0;

    Entity currentNode = INVALID_ENTITY;

    int winTimer = -1;
    int loseTimer = -1;

    std::string gameLevelName = "test";

    int level = -1; //Mañana. setear nivel al doble click en nodo en menu. si nivel ganado winnedLevel = level, cuando loadMenu 
    int winnedLevel = -1; //for(node : node) if(node.value == winnedLevel) node.type = None, node.adyacent.type = good... Si entras a un nodo None level = -1;
    // Para no volver a poner verde, obviamente antes del for para revisar winnedLevel revisar que winnedLevel no sea -1;
};

struct ConfigState{
    bool lineForces = true;
    bool nodeRepulsion = true;
    bool centerAttraction = true;
};

struct EditorState{
    Entity holdEntity = INVALID_ENTITY;
    Entity tempLine = INVALID_ENTITY;

    std::unordered_map<Entity, bool> openedWindows = {};

    Graph* graph = nullptr;

    WindowData windowData = {{100, 100},  1, {0, 0}};

    ImFont* MonoFont = nullptr;
    ImFont* MonoFontBold = nullptr;
    float MonoFontSize = 18.f;

    ConfigState config;
    GameState game;
};

extern EditorState editorState;

struct Forces{
    float centerAttraction = .01f;
    float nodeRepulsionForce = 4.5f;
    float nodeRepulsionRadius = Style::NodeSize * 10;
    float lineForce = .04f;
    float lineCenter = 90.f;
};

extern Forces forces;

class StateMachine;
extern StateMachine stateMachine;
#pragma once
#include "ThING/types/apiTypes.h"
#include "auxiliar/style.h"
#include "graph/graph.h"
#include "auxiliar/window.h"
#include "imgui.h"

struct GameState{
    int points = 0;
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

    GameState game;
};

extern EditorState editorState;

struct Forces{
    float centerAttraction = .99f;
    float nodeRepulsionForce = 4.5f;
    float nodeRepulsionRadius = Style::NodeSize * 10;
    float lineForce = .04f;
    float lineCenter = 90.f;
};

extern Forces forces;

class StateMachine;
extern StateMachine stateMachine;
#pragma once
#include "graph/graph.h"
#include "auxiliar/window.h"

struct EditorState{
    bool leftHold = false;
    bool leftDrag = false;
    Entity holdEntity = INVALID_ENTITY;

    std::unordered_map<Entity, bool> openedWindows = {};
    bool rightHold = false;
    bool rightDrag = false;

    Graph* graph = nullptr;

    WindowData windowData = {{100, 100},  1, {0, 0}};
};

extern EditorState editorState;
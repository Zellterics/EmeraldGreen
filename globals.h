#pragma once
#include "graph/graph.h"
#include "auxiliar/window.h"
#include "imgui.h"


enum class StateM{
    Idle,
    WaitLeftIdle,
    WaitRightIdle,
    DraggingLine,
    DraggingNode,
    PanningCamera
};

inline const char* StateMToString(StateM s){
    switch(s){
        case StateM::Idle:             return "Idle";
        case StateM::WaitLeftIdle:     return "WaitLeftIdle";
        case StateM::WaitRightIdle:    return "WaitRightIdle";
        case StateM::DraggingLine:     return "DraggingLine";
        case StateM::DraggingNode:     return "DraggingNode";
        case StateM::PanningCamera:    return "PanningCamera";
        default:                       return "UnknownState";
    }
}

struct EditorState{
    StateM stateM = StateM::Idle;
    Entity holdEntity = INVALID_ENTITY;

    std::unordered_map<Entity, bool> openedWindows = {};

    Graph* graph = nullptr;

    WindowData windowData = {{100, 100},  1, {0, 0}};

    ImFont* MonoFont = nullptr;
    ImFont* MonoFontBold = nullptr;
    float MonoFontSize = 18.f;
};

extern EditorState editorState;
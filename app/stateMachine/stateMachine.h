#pragma once

#include <string>

struct EditorState;
class Graph;
namespace ThING {
    class API;
}

enum class StateM{
    Idle,
    WaitLeftIdle,
    WaitRightIdle,
    DraggingLine,
    DraggingNode,
    DraggingRight,
    PlayingAnimation,

    GameIdle,
    GameWaitLeftIdle,
    GameWaitRightIdle, // Make just one version with parameters (StateM returnState) to go back to the respective IdleStage Same witg Menu
    GameDraggingRight, // if returnState == GameIdle; menuWindow(api, resume = true); for resume option [I don't remember what this means]

    MenuIdle,

    Menu,
    PanningCamera,
};

class StateMachine {
public:
    StateMachine();
    void bind(ThING::API& api, EditorState& editorState);
    std::string stateToString();
    StateM& getState() {return currentState;}
    StateM& getReturnState() {return returnState;}
    void update();
private:
    void selectNode();
    void deselectNode();

    void idle();
    void waitLeftIdle();
    void waitRightIdle();
    void dragginLine();
    void draggingNode();
    void panningCamera();
    void draggingRight();
    void playingAnimation();

    void gameIdle();
    void gameWaitLeftIdle();
    void gameWaitRightIdle();
    void gameDraggingRight();

    void menuIdle();

    void menu();
    StateM returnState;
    StateM currentState;

    ThING::API* api;
    EditorState* editorState;
};
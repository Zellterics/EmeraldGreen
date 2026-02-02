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
    PanningCamera,
    DraggingRight,
    PlayingAnimation
};

class StateMachine {
public:
    StateMachine();
    void bind(ThING::API& api, EditorState& editorState);
    std::string stateToString();
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
    StateM currentState;

    ThING::API* api;
    EditorState* editorState;
};
#include "glm/ext/vector_float2.hpp"
#include "imgui.h"
struct WindowSize{
    int width;
    int height;
    bool operator==(const WindowSize& other) const {
        if(other.height == height && other.width == width){
            return true;
        }
        return false;
    }
    bool operator!=(const WindowSize& other) const {
        if(other.height == height && other.width == width){
            return false;
        }
        return true;
    }
};

struct WindowData{
    WindowSize size;
    float zoom;
    glm::vec2 offset;
};

glm::vec2 mousePosition(WindowData windowData){
    ImVec2 tempPosition = ImGui::GetMousePos();
    return {
        ((tempPosition.x - ((float)windowData.size.width / 2)) / windowData.zoom) + windowData.offset.x,
        ((tempPosition.y - ((float)windowData.size.height / 2)) / windowData.zoom) + windowData.offset.y
    };
}
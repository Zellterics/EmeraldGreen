#pragma once
#include "ThING/api.h"
#include "ThING/types/apiTypes.h"
#include "glm/fwd.hpp"

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

glm::vec2 mousePosition(WindowData windowData);

Entity hitEntity(ThING::API& api, WindowData windowData);
Entity hitEntity(ThING::API& api, WindowData windowData, glm::vec2 point);
Entity hitEntity(ThING::API& api, WindowData windowData, float margin);

template<typename T>
inline T worldToImGui(glm::vec2 pos, WindowData& windowData){
    return {
        (pos.x - windowData.offset.x) * windowData.zoom + windowData.size.width  * 0.5f,
        (pos.y - windowData.offset.y) * windowData.zoom + windowData.size.height * 0.5f
    };
}

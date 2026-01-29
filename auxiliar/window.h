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
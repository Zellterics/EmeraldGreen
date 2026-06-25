#pragma once
#include "ThING/api.h"
#include "ThING/types/apiTypes.h"
#include "glm/fwd.hpp"
#include "imgui.h"
#include "style.h"

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

bool openTree(ThING::API& api, std::string id);
bool beginWindow(ThING::API& api, std::string id);
template<typename... Args>
bool beginWindow(ThING::API& api, std::string id, Args... args){
    static std::unordered_map<std::string, bool> wasOpen;
    if(!wasOpen.contains(id)){
        wasOpen[id] = ImGui::Begin(id.c_str(), std::forward<Args>(args)...);
        return wasOpen[id];
    }
    bool isOpen = ImGui::Begin(id.c_str(), std::forward<Args>(args)...);

    if (wasOpen[id] && !isOpen) {
        api.playAudio(Style::Audio::CloseTree);
    }
    if(!wasOpen[id] && isOpen){
        api.playAudio(Style::Audio::OpenTree);
    }
    wasOpen[id] = isOpen;
    return isOpen;
}

template<typename T>
inline T worldToImGui(glm::vec2 pos, WindowData& windowData){
    return {
        (pos.x - windowData.offset.x) * windowData.zoom + windowData.size.width  * 0.5f,
        (pos.y - windowData.offset.y) * windowData.zoom + windowData.size.height * 0.5f
    };
}

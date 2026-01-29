#include "imgui.h"
#include "window.h"

glm::vec2 mousePosition(WindowData windowData){
    static int lastFrame = -1;
    int currentFrame = ImGui::GetFrameCount();
    static glm::vec2 cache;
    if(currentFrame == lastFrame){
        return cache;
    }
    lastFrame = currentFrame;
    ImVec2 tempPosition = ImGui::GetMousePos();
    cache = {
        ((tempPosition.x - ((float)windowData.size.width / 2)) / windowData.zoom) + windowData.offset.x,
        ((tempPosition.y - ((float)windowData.size.height / 2)) / windowData.zoom) + windowData.offset.y
    };
    return cache;
}

Entity hitEntity(ThING::API& api, WindowData windowData){
    static int lastFrame = -1;
    int currentFrame = ImGui::GetFrameCount();
    static Entity cache;
    if(currentFrame == lastFrame){
        return cache;
    }
    lastFrame = currentFrame;

    cache = INVALID_ENTITY;
    std::span circleInstances = api.getInstanceVector(InstanceType::Circle);
    for (uint32_t i = 0; i < circleInstances.size(); i++) {
        const InstanceData& c = circleInstances[i];

        if(!c.alive) continue;

        glm::vec2 distance = mousePosition(windowData) - c.position;
        if (glm::dot(distance, distance) <= c.scale.x * c.scale.x) {
            cache = {i, InstanceType::Circle};
            break;
        }
    }
    if(api.exists(cache)){
        return cache;
    }
    return INVALID_ENTITY;
}
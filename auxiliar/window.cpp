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

Entity hitEntity(ThING::API& api, WindowData windowData, glm::vec2 point){
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

        glm::vec2 distance = point - c.position;
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

Entity hitEntity(ThING::API& api, WindowData windowData, float margin){
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
        const float radius = c.scale.x + margin;
        if(!c.alive) continue;

        glm::vec2 distance = mousePosition(windowData) - c.position;
        if (glm::dot(distance, distance) <= radius * radius) {
            cache = {i, InstanceType::Circle};
            break;
        }
    }
    if(api.exists(cache)){
        return cache;
    }
    return INVALID_ENTITY;
}

bool openTree(ThING::API& api, std::string id){
    static std::unordered_map<std::string, bool> wasOpen;
    if(!wasOpen.contains(id)){
        wasOpen[id] = ImGui::TreeNode(id.c_str());
        return wasOpen[id];
    }
    bool isOpen = ImGui::TreeNode(id.c_str());

    if (wasOpen[id] && !isOpen) {
        api.playAudio(Style::Audio::CloseTree);
    }
    if(!wasOpen[id] && isOpen){
        api.playAudio(Style::Audio::OpenTree);
    }
    wasOpen[id] = isOpen;
    return isOpen;
}

bool beginWindow(ThING::API& api, std::string id){
    static std::unordered_map<std::string, bool> wasOpen;
    if(!wasOpen.contains(id)){
        wasOpen[id] = ImGui::Begin(id.c_str());
        return wasOpen[id];
    }
    bool isOpen = ImGui::Begin(id.c_str());

    if (wasOpen[id] && !isOpen) {
        api.playAudio(Style::Audio::CloseTree);
    }
    if(!wasOpen[id] && isOpen){
        api.playAudio(Style::Audio::OpenTree);
    }
    wasOpen[id] = isOpen;
    return isOpen;
}
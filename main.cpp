#include "ThING/core.h"
#include "ThING/types/apiTypes.h"
#include "ThING/types/enums.h"
#include "ThING/types/renderData.h"
#include "glm/fwd.hpp"
#include "graph/graph.h"
#include <ThING/api.h>
#include <cstdint>
#include <imgui.h>
#include "consts.h"

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

glm::vec2 windowPosition(WindowSize windowSize, float zoom, glm::vec2 offset){
    ImVec2 tempPosition = ImGui::GetMousePos();
    return {
        ((tempPosition.x - ((float)windowSize.width / 2)) / zoom) + offset.x,
        ((tempPosition.y - ((float)windowSize.height / 2)) / zoom) + offset.y
    };
}

Entity hitEntity(ThING::API& api, WindowSize windowSize, float zoom, glm::vec2 offset){
    Entity e = INVALID_ENTITY;
    std::span circleInstances = api.getInstanceVector(InstanceType::Circle);
    for (uint32_t i = 0; i < circleInstances.size(); i++) {
        const InstanceData& c = circleInstances[i];

        if(!c.alive) continue;

        glm::vec2 distance = windowPosition(windowSize, zoom, offset) - c.position;
        if (glm::dot(distance, distance) <= c.scale.x * c.scale.x) {
            e = {i, InstanceType::Circle};
            break;
        }
    }
    if(api.exists(e)){
        return e;
    }
    return INVALID_ENTITY;
}
void ApplyNodeEditorStyle()
{
    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowRounding = 12.0f;
    style.FrameRounding  = 8.0f;
    style.PopupRounding  = 10.0f;
    style.ScrollbarRounding = 10.0f;
    style.GrabRounding = 8.0f;

    style.WindowPadding = ImVec2(16, 14);
    style.FramePadding  = ImVec2(10, 6);
    style.ItemSpacing   = ImVec2(10, 10);

    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize  = 0.0f;

    ImVec4* c = style.Colors;

    c[ImGuiCol_WindowBg]  = ImVec4(0.04f, 0.04f, 0.04f, 0.97f);
    c[ImGuiCol_ChildBg]   = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
    c[ImGuiCol_PopupBg]   = ImVec4(0.06f, 0.06f, 0.06f, 0.98f);

    c[ImGuiCol_Border] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);

    c[ImGuiCol_FrameBg]        = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
    c[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    c[ImGuiCol_FrameBgActive]  = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);

    c[ImGuiCol_Text]        = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    c[ImGuiCol_TextDisabled]= ImVec4(0.55f, 0.55f, 0.55f, 1.00f);

    c[ImGuiCol_Button]        = ImVec4(0.30f, 0.60f, 1.00f, 0.85f);
    c[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.65f, 1.00f, 1.00f);
    c[ImGuiCol_ButtonActive]  = ImVec4(0.25f, 0.55f, 0.95f, 1.00f);

    c[ImGuiCol_Header]        = ImVec4(0.30f, 0.60f, 1.00f, 0.35f);
    c[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.60f, 1.00f, 0.55f);
    c[ImGuiCol_HeaderActive]  = ImVec4(0.30f, 0.60f, 1.00f, 0.75f);

    c[ImGuiCol_CheckMark]     = ImVec4(0.30f, 0.60f, 1.00f, 1.00f);
    c[ImGuiCol_SliderGrab]    = ImVec4(0.30f, 0.60f, 1.00f, 1.00f);
    c[ImGuiCol_SliderGrabActive]
                               = ImVec4(0.40f, 0.70f, 1.00f, 1.00f);

    c[ImGuiCol_TitleBg]        = ImVec4(0.03f, 0.03f, 0.03f, 1.00f);
    c[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
}


void updateCallback(ThING::API& api, FPSCounter fps){
    static bool first = true;

    static bool hold = false;
    static bool drag = false;
    static Entity holdEntity = INVALID_ENTITY;

    static std::unordered_map<Entity, bool> openedWindows;
    static bool rightHold = false;
    static bool rightDrag = false;

    static float zoom = 1;
    static glm::vec2 offset = {0,0};

    static Graph graph(api);

    static Entity tempLine = INVALID_ENTITY;
    WindowSize windowSize;
    api.getWindowSize(&windowSize.width, &windowSize.height);

    if(first){
        ApplyNodeEditorStyle();
        first = false;
    }

    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureMouse){
        if(ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !(rightHold || rightDrag)){
            holdEntity = hitEntity(api, windowSize, zoom, offset);
            hold = true;
        }
        if(ImGui::IsMouseDragging(ImGuiMouseButton_Left) && hold){
            if(holdEntity == INVALID_ENTITY){
                
                ImVec2 delta = ImGui::GetIO().MouseDelta;
                offset.x -= delta.x / zoom;
                offset.y -= delta.y / zoom;
                api.setOffset(offset);
            } else {
                if(tempLine == INVALID_ENTITY){
                    tempLine = api.addLine(api.getInstance(holdEntity).position, windowPosition(windowSize, zoom, offset), Color::LineWidth);
                    api.getLine(tempLine).color = Color::Line;
                    api.getLine(tempLine).outlineColor = Color::Outline;
                    api.getLine(tempLine).outlineSize = Color::OutlineWidth;
                    api.getLine(tempLine).objectID = 1;
                } else {
                    api.getLine(tempLine).point2 = windowPosition(windowSize, zoom, offset);
                }
            }
            drag = true;
        }
        if(ImGui::IsMouseReleased(ImGuiMouseButton_Left) && hold){
            if(drag){
                if(api.exists(holdEntity)){
                    graph.connect(holdEntity, hitEntity(api, windowSize, zoom, offset));
                }
            } else {
                Entity e = graph.addNode(windowPosition(windowSize, zoom, offset));
            }
            if(tempLine != INVALID_ENTITY){
                api.deleteInstance(tempLine);
                tempLine = INVALID_ENTITY;
            }
            drag = false;
            hold = false;
        }
        if(!(drag || hold)){
            if(ImGui::IsMouseClicked(ImGuiMouseButton_Right)){
                holdEntity = hitEntity(api, windowSize, zoom, offset);
                rightHold = true;
            }
        }
        if(ImGui::IsMouseDragging(ImGuiMouseButton_Right) && rightHold){
            if(holdEntity != INVALID_ENTITY){
                api.getInstance(holdEntity).position = windowPosition(windowSize, zoom, offset);
                graph.update();
            }
            rightDrag = true;
        }
        if(ImGui::IsMouseReleased(ImGuiMouseButton_Right) && rightHold){
            
            if(!rightDrag && holdEntity != INVALID_ENTITY){
                openedWindows[holdEntity] = true;
            }
            rightHold = false;
            rightDrag = false;
        }
    }
    
    std::span<InstanceData> circleInstances = api.getInstanceVector(InstanceType::Circle);
    for(uint32_t i = 0; i < circleInstances.size(); i++){
        Entity e = {i, InstanceType::Circle};
        if(openedWindows.contains(e) && openedWindows[e]){
            ImGui::SetNextWindowPos(ImGui::GetMousePos(), ImGuiCond_Appearing);
            ImGui::SetNextWindowSize(ImVec2(250, 150), ImGuiCond_Once);
            ImGui::Begin( ("Circle " + std::to_string(i)).c_str(), &openedWindows[e]);

            ImGui::PushID(i);

            if(ImGui::Button("Delete")){
                graph.deleteNode(e);
                graph.update();
                openedWindows[e] = false;
            }

            ImGui::PopID();
            ImGui::End();
        }
    }
    float scroll = ImGui::GetIO().MouseWheel;
    
    glm::vec2 pos = windowPosition(windowSize, zoom, offset);
    if(scroll != 0){
        scroll /= 20;
        float oldZoom = zoom;
        zoom *= 1 + scroll;
        offset += (pos - offset) * (1.f - (oldZoom / zoom));
        api.setZoom(zoom);
        api.setOffset(offset);
    }
}

int main(){
    ThING::API api;
    api.setBackgroundColor(Color::Background);
    api.setUpdateCallback(updateCallback);
    api.run();
}
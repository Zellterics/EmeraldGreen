#include "ThING/types/apiTypes.h"
#include "graph/graph.h"
#include <ThING/api.h>
#include <imgui.h>

void updateCallback(ThING::API& api, FPSCounter fps){
    static Graph graph(api);
    int width, height;
    api.getWindowSize(&width, &height);
    width /= 2;
    height /= 2;
    if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)){
        ImVec2 tempPosition = ImGui::GetMousePos();
        glm::vec2 pos = {
            tempPosition.x - width,
            tempPosition.y - height
        };
        Entity e = graph.addNode(pos);
        graph.connect(e, graph.first());
        //graph.update();
    }
}

int main(){
    ThING::API api;
    api.setBackgroundColor({.1,.1,.1,1});
    api.setUpdateCallback(updateCallback);
    api.run();
}
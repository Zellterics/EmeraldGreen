#include "app/auxiliar/style.h"
#include "glm/fwd.hpp"
#include "app/globals.h"
#include <ThING/api.h>

#include "app/graph/graph.h"

#include "app/callbacks/ui.h"
#include "app/callbacks/update.h"

int main(){
    ThING::API api(ApiFlags_UpdateCallbackFirst);
    editorState.graph = new Graph(api);
    stateMachine.bind(api, editorState);
    api.setBackgroundColor(Style::Color::Background);
    api.setUpdateCallback(updateCallback);
    api.setUICallback(uiCallback);
    imGuiInitialize();
    api.run();
}// MAKE BAU BAU HAPPY, WHEN RESET KEEP NODE POSITION THE SAME
// Make pickPhysical device use the best graphics card not just the available
// Still bad in integrated, optimize further the outline shader
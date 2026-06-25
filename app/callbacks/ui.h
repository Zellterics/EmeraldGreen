#pragma once
#include <ThING/api.h>
#include "../stateMachine/stateMachine.h"

void uiCallback(ThING::API& api, FPSCounter& fps);

void imGuiInitialize();
void nodeWindows(ThING::API& api);
void debugWindow(ThING::API& api);
void scrollZoom(ThING::API& api);
void menuWindow(ThING::API& api);
void drawNodeValues(ThING::API& api);
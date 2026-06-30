#pragma once
#include <ThING/api.h>

void updateCallback(ThING::API& api, FPSCounter& fps);

void handleFinishTimer(ThING::API& api, float deltaTime);
void selectCurrentNode(ThING::API& api);
void updateGraph();
void navigationLogic(ThING::API& api);
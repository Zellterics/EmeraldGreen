#pragma once
#include <ThING/api.h>
#include "../stateMachine/stateMachine.h"

void nodeWindows(ThING::API& api);
void debugWindow(ThING::API& api);
void scrollZoom(ThING::API& api);
void menuWindow(ThING::API& api);

void loadLevel(ThING::API& api, std::string filename);
void saveLevel(ThING::API& api, std::string filename);
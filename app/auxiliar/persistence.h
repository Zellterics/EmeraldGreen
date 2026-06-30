#pragma once
#include <ThING/api.h>

bool loadLevel(ThING::API& api, std::string filename);
void saveLevel(ThING::API& api, std::string filename);
void loadMenu(ThING::API& api);

void restartLevel(ThING::API& api);
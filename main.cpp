#include <ThING/api.h>

void updateCallback(ThING::API& api, FPSCounter fps){
    static bool first = true;
    if(first)
        api.addCircle({1,1}, 30, {1,0,1,1});
    first = false;
}

int main(){
    ThING::API api;
    api.setUpdateCallback(updateCallback);
    api.run();
}
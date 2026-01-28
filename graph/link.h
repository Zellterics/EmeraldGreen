#pragma once

#include <ThING/types/renderData.h>
#include <ThING/types/apiTypes.h>

class Link{
public:
    Link(Entity other, Entity line) : other(other), line(line){}
    inline Entity viewConnection() const {return other;}
    inline Entity viewLine() const {return line;}
private:
    Entity line;
    Entity other;
};
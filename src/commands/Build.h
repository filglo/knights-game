#pragma once

#include "Command.h"
#include "../GameObject.h"

class Build : public Command {
public:
    Build(int unit, ObjectType object);
    std::string Serialize();
    void Execute(Game& game);
private:
    int unitID;
    ObjectType unitToBuild;
};
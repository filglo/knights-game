#pragma once

#include "Command.h"

class Move : public Command {
public:
    Move(int unit, Coords c);
    std::string Serialize();
    void Execute(Game& game);
private:
    int unitID;
    Coords coords;
};
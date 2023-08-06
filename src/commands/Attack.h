#pragma once

#include "Command.h"

class Attack : public Command {
public:
    Attack(int unit, int attackedUnit);
    std::string Serialize();
    void Execute(Game& game);
private:
    int unitID;
    int attackedUnitID;
};
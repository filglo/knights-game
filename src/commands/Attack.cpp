#include "Attack.h"

#include <sstream>

Attack::Attack(int unit, int attackedUnit)
    : unitID(unit)
    , attackedUnitID(attackedUnit)
{
}

std::string Attack::Serialize() {
    std::stringstream output;
    output << unitID << " A " << attackedUnitID << '\n';
    return output.str();
}

void Attack::Execute(Game& game) {
    game.Attack(unitID, attackedUnitID);
}
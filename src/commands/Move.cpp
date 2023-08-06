#include "Move.h"

#include <sstream>

Move::Move(int unit, Coords c)
    : unitID(unit)
    , coords(c)
{
}

std::string Move::Serialize() {
    std::stringstream output;
    output << unitID << " M " << coords.first << ' ' << coords.second << '\n';
    return output.str();
}

void Move::Execute(Game& game) {
    game.Move(unitID, coords);
}
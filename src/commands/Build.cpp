#include "Build.h"

#include <sstream>

Build::Build(int unit, ObjectType object)
    : unitID(unit)
    , unitToBuild(object)
{
}

std::string Build::Serialize() {
    std::stringstream output;
    output << unitID << " B " << GameConstants::GetIdentifierFromType(unitToBuild) << '\n';
    return output.str();
}

void Build::Execute(Game& game) {
    game.Build(unitID, unitToBuild);
}
#include "GameConstants.h"
#include <stdexcept>

ObjectType GameConstants::GetTypeFromIdentifier(char id) {
    switch (id)
    {
        case 'K':
            return ObjectType::KNIGHT;
        case 'S':
            return ObjectType::SWORDSMAN;
        case 'A':
            return ObjectType::ARCHER;
        case 'P':
            return ObjectType::PIKEMAN;
        case 'R':
            return ObjectType::RAM;
        case 'C':
            return ObjectType::CATAPULT;
        case 'W':
            return ObjectType::WORKER;
        case 'B':
            return ObjectType::BASE;
        default:
            throw std::invalid_argument("Invalid unit identifier.");
            break;
    }
    return ObjectType::BASE;
}
char GameConstants::GetIdentifierFromType(ObjectType type) {
    return std::get<0>(unitConfig.at(type));
}
MapTile GameConstants::GetMapTileFromIdentifier(char id) {
    switch (id)
    {
        case '1':
            return MapTile::BASE_PLAYER;
        case '2':
            return MapTile::BASE_OPPONENT;
        case '6':
            return MapTile::MINE;
        case '9':
            return MapTile::OBSTACLE;
        case '0':
            return MapTile::EMPTY;
        default:
            throw std::invalid_argument("Invalid map token.");
            break;
    }
    return MapTile::EMPTY;
}

int GameConstants::GetUnitPrice(ObjectType type)
{
    return std::get<3>(unitConfig.at(type));
}

int GameConstants::GetUnitBaseMoves(ObjectType type)
{
    return std::get<4>(unitConfig.at(type));
}

int GameConstants::GetUnitBaseDurability(ObjectType type)
{
    return std::get<1>(unitConfig.at(type));
}

int GameConstants::GetUnitTurnsToBuild(ObjectType type)
{
    return std::get<2>(unitConfig.at(type));
}

int GameConstants::GetUnitRange(ObjectType type)
{
    return std::get<5>(unitConfig.at(type));
}

int GameConstants::GetUnitDamageToTarget(ObjectType type, ObjectType target)
{
    return std::get<6>(unitConfig.at(type)).at(target);
}

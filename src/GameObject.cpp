#include "GameObject.h"

#include "GameMap.h"
#include <sstream>

GameObject::GameObject(ObjectType t, int id, Coords pos, int playerID, int durability)
    : coords(pos)
    , type(t)
    , id(id)
    , playerID(playerID)
    , durability(durability)
{   
}

ObjectType GameObject::GetType() const {
    return type;
}

int GameObject::GetPlayerID() const {
    return playerID;
}

Coords GameObject::GetPos() const {
    return coords;
}

int GameObject::GetID() const {
    return id;
}

int GameObject::GetDurability() const {
    return durability;
}

bool GameObject::IsDestroyed() const {
    return durability <= 0;
}

void GameObject::Damage(int damage) {
    if(durability <= 0)
        throw std::logic_error("Destroyed unit cannot be damaged.");
    durability -= damage;
    if(durability <= 0)
        onDestroy(GetID(), GetPlayerID());
}

void GameObject::AddCallbackOnDestroy(std::function<void(int, int)> f) {
    onDestroy = f;
}


Base::Base(int id, Coords pos, int playerID, int durability, ObjectType buildType, bool building)
    : GameObject(ObjectType::BASE, id, pos, playerID, durability)
    , turnsToBuild(-1)
    , isBuilding(building)
    , unitBuildType(buildType)
{
}

std::string Base::Serialize(int playerID) const {
    std::stringstream baseString;

    baseString << (GetPlayerID() == playerID ? "P " : "E ");
    baseString << GameConstants::GetIdentifierFromType(GetType()) << ' ';
    baseString << GetID() << ' ';
    baseString << coords.first << ' ' << coords.second << ' ';
    baseString << GetDurability() << ' ';
    baseString << (IsBuilding() ? GameConstants::GetIdentifierFromType(GetUnitBuildType()) : '0') << '\n';

    return baseString.str();
}

int Base::GetTurnsToBuild() const {
    return turnsToBuild;
}

ObjectType Base::GetUnitBuildType() const {
    return unitBuildType;
}

bool Base::IsBuilding() const {
    return isBuilding;
}

void Base::Build(ObjectType buildType) {
    if(buildType == ObjectType::BASE)
        throw std::invalid_argument("Cannot build unit of type base.");
    if(isBuilding)
        throw std::invalid_argument("Base is already building unit.");
    unitBuildType = buildType;
    isBuilding = true;
    turnsToBuild = GameConstants::GetUnitTurnsToBuild(buildType);
}

void Base::AddCallbackOnBuild(std::function<void(ObjectType, Coords coords, int playerID)> f) {
    onBuild = f;
}

void Base::TakeTurn() {
    if(turnsToBuild > 0) {
        turnsToBuild--;
        if(turnsToBuild == 0) {
            onBuild(unitBuildType, coords, GetPlayerID());
            isBuilding = false;
        }
    }

}

void Base::MoveTo(Coords pos) {
    throw std::logic_error("Base cannot move");
}

Unit::Unit(ObjectType t, int id, Coords pos, int playerID, int durability)
    : GameObject(t, id, pos, playerID, durability)
    , hasAttacked(false)
    , remainingMoves(GameConstants::GetUnitBaseMoves(t))
{   
}

std::string Unit::Serialize(int playerID) const {
    std::stringstream unitString;

    unitString << (GetPlayerID() == playerID ? "P " : "E ");
    unitString << GameConstants::GetIdentifierFromType(GetType()) << ' ';
    unitString << GetID() << ' ';
    unitString << coords.first << ' ' << coords.second << ' ';
    unitString << GetDurability() << '\n';

    return unitString.str();
}

void Unit::MoveTo(Coords pos) {
    remainingMoves -= GameMap::Distance(*this, pos);
    if(remainingMoves < 0)
        throw std::logic_error("Unit doesn't have enough movement points.");
}

void Unit::Attack() {
    if(hasAttacked || remainingMoves < 1)
        throw std::logic_error("Unit cannot attack.");
    hasAttacked = true;
    remainingMoves -= 1;
}

int Unit::GetMoves() const {
    return remainingMoves;
}

bool Unit::HasAttacked() const {
    return hasAttacked;
}

void Unit::TakeTurn() {
    hasAttacked = false;
    remainingMoves = GameConstants::GetUnitBaseMoves(GetType());
}

#pragma once

#include "GameConstants.h"
#include <vector>
#include <string>
#include <functional>

typedef std::pair<int, int> Coords;

class GameObject {

public:
    GameObject(ObjectType t, int id, Coords pos, int playerID, int durability);
    virtual std::string Serialize(int playerID) const = 0;
    ObjectType GetType() const;
    int GetPlayerID() const;
    Coords GetPos() const;
    int GetID() const;
    int GetDurability() const;
    bool IsDestroyed() const;
    void Damage(int damage);
    void AddCallbackOnDestroy(std::function<void(int, int)> f);
    virtual void TakeTurn() {};
    virtual void MoveTo(Coords pos) {};
    virtual void Attack() {};

protected:
    Coords coords;

private:
    std::function<void(int, int)> onDestroy;
    ObjectType type;
    int id;
    int playerID;
    int durability;
};

class Base : public GameObject {
public:
    Base(int id, Coords c, int playerID, int durability, ObjectType buildType, bool building);
    std::string Serialize(int playerID) const;
    int GetTurnsToBuild() const;
    ObjectType GetUnitBuildType() const;
    bool IsBuilding() const;
    void Build(ObjectType buildType);
    void AddCallbackOnBuild(std::function<void(ObjectType, Coords coords, int)> f);
    void TakeTurn();
    void MoveTo(Coords pos);

private:
    std::function<void(ObjectType, Coords coords, int)> onBuild;
    int turnsToBuild;
    bool isBuilding;
    ObjectType unitBuildType;
};

class Unit : public GameObject {
public:
    Unit(ObjectType t, int id, Coords c, int playerID, int durability);
    std::string Serialize(int playerID) const;
    void MoveTo(Coords pos);
    void Attack();
    int GetMoves() const;
    bool HasAttacked() const;
    void TakeTurn();

private:
    bool hasAttacked;
    int remainingMoves;
};
#pragma once

#include "GameMap.h"
#include "GameObject.h"

enum class GameWinState {IN_PROGRESS, TIE, PLAYER1_WIN, PLAYER2_WIN, PLAYER1_TIMEOUT, PLAYER2_TIMEOUT, PLAYER1_ERROR, PLAYER2_ERROR};

class Game {
public:
    Game(const char* map, const char* statusP1, const char* statusP2);
    GameWinState Turn();
    void Attack(int unitID, int targetID);
    void Build(int unitID, ObjectType type);
    void Move(int unitID, Coords coords);
    std::string SerializeStatus(long int gold, int playerID) const;
    std::vector<const GameObject*> GetObjectsAtPos(Coords coords) const;
    
    const Base* GetPlayerBase(int playerID) const;
    std::vector<const Unit*> GetPlayerUnits(int playerID) const;
    std::vector<const GameObject*> GetPlayerObjects(int playerID) const;
    bool IsValidPlacement(const Unit* unit, Coords coords) const;
    int CountObjectsForPlayer(int playerID) const;
    int GetTurnNumber() const;
    int GetPlayerGold(int playerID) const;
    const GameMap& GetMap() const;

private:
    void DeserializeStatus(const char* status, int playerID);
    void DeserializeGold(const char* status, int playerID);
    void PayForUnit(int amount, int playerID);
    GameObject* GetObjectWithID(int id);

    long int playerOneGold, playerTwoGold;
    int turnNumber;
    GameWinState gameWinState;
    int nextUnitID;
    GameMap map;
    std::vector<std::unique_ptr<GameObject>> objects;
};
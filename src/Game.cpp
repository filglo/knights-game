#include "Game.h"

#include <fstream>
#include <string>
#include <sstream>

Game::Game(const char* mapFilename, const char* statusP1, const char* statusP2)
    : playerOneGold(2500)
    , playerTwoGold(2500)
    , turnNumber(0)
    , gameWinState(GameWinState::IN_PROGRESS)
    , nextUnitID(0)
    , map(mapFilename)
{
    DeserializeStatus(statusP1, 1);
    DeserializeGold(statusP2, 2);
}

GameWinState Game::Turn() {
    // Destroy dead units
    for(size_t i = 0; i < objects.size(); i++) {
        if(objects[i]->IsDestroyed()) {
            if(i != objects.size()-1)
                objects[i] = std::move(objects.back());
            objects.pop_back();
        }
    }

    // Check win conditions
    if(gameWinState != GameWinState::IN_PROGRESS)
        return gameWinState;
    if(turnNumber == 2000-1) {
        int unitsP1 = CountObjectsForPlayer(true);
        int unitsP2 = CountObjectsForPlayer(false);
        if(unitsP1 > unitsP2)
            return GameWinState::PLAYER1_WIN;
        else if(unitsP2 > unitsP1)
            return GameWinState::PLAYER2_WIN;
        else
            return GameWinState::TIE;
    }

    // Update game objects
    auto objSize = objects.size();
    for(size_t i = 0; i < objSize; i++) {
        objects[i]->TakeTurn();
    }

    // Add gold to players according to number of workers in mines
    int workerCountP1 = 0;
    int workerCountP2 = 0;
    for(auto pos : map.GetMinePositions()) {
        auto units = GetObjectsAtPos(pos);
        for(auto unit : units) {
            if(unit->GetType() == ObjectType::WORKER) {
                if(unit->GetPlayerID() == 1)
                    workerCountP1++;
                else
                    workerCountP2++;
            }
        }
    }
    playerOneGold += workerCountP1 * 50;
    playerTwoGold += workerCountP2 * 50;

    turnNumber++;
    return GameWinState::IN_PROGRESS;
}

void Game::DeserializeStatus(const char* status, int playerID) {
    std::ifstream statusFile(status);

    char ownership;
    char type;
    int id, posX, posY, durability;
    char buildType;
    if(playerID == 1)
        statusFile >> playerOneGold;
    else
        statusFile >> playerTwoGold;
    while(statusFile >> ownership >> type >> id >> posX >> posY >> durability) {
        if(id >= nextUnitID)
            nextUnitID = id+1;
        if(type == 'B') {
            statusFile >> buildType;
            bool isBuilding = false;
            ObjectType bType;
            if(buildType == '0') {
                bType = ObjectType::BASE;
            } else {
                isBuilding = true;
                bType = GameConstants::GetTypeFromIdentifier(buildType);
            }

            auto baseDestroyed = [this](int unitID, int playerID){
                gameWinState = playerID == 1 ? GameWinState::PLAYER2_WIN : GameWinState::PLAYER1_WIN;
            };
            auto unitBuilt = [this](ObjectType type, Coords coords, int playerID) {
                objects.push_back(std::make_unique<Unit>(
                            type, nextUnitID, coords, playerID, GameConstants::GetUnitBaseDurability(type)));
                nextUnitID++;
            };

            auto base = std::make_unique<Base>(id, std::make_pair(posX, posY), ownership == 'P' ? 1 : 2, durability, bType, isBuilding);
            base->AddCallbackOnDestroy(baseDestroyed);
            base->AddCallbackOnBuild(unitBuilt);
            objects.emplace_back(std::move(base));
        } else {
            objects.push_back(std::make_unique<Unit>(GameConstants::GetTypeFromIdentifier(type), id, std::make_pair(posX, posY), ownership == 'P' ? 1 : 2, durability));
        }
    }
    statusFile.close();
}

void Game::DeserializeGold(const char *status, int playerID) {
    std::ifstream statusFile(status);

    if(playerID == 1)
        statusFile >> playerOneGold;
    else
        statusFile >> playerTwoGold;
    statusFile.close();
}

void Game::Attack(int unitID, int targetID) {
    auto thisUnit = GetObjectWithID(unitID);
    auto attackedUnit = GetObjectWithID(targetID);
    if(thisUnit == nullptr || attackedUnit == nullptr)
        throw std::logic_error("Invalid attack parameter");
    if(attackedUnit->IsDestroyed())
        throw std::logic_error("Unit is already destroyed");
    thisUnit->Attack();
    auto damageValue = GameConstants::GetUnitDamageToTarget(thisUnit->GetType(), attackedUnit->GetType());
    attackedUnit->Damage(damageValue);
}

void Game::Build(int unitID, ObjectType type) {
    auto base = GetObjectWithID(unitID);
    PayForUnit(GameConstants::GetUnitPrice(type), base->GetPlayerID());
    dynamic_cast<Base*>(base)->Build(type);
}

void Game::Move(int unitID, Coords coords) {
    auto object = GetObjectWithID(unitID);
    if(object == nullptr)
        throw std::logic_error("Invalid unit ID.");
    if(object->GetPos() == coords)
        throw std::logic_error("Cannot move 0 distance.");
    if(!IsValidPlacement(dynamic_cast<const Unit*>(object), coords))
        throw std::logic_error("Invalid unit placement");
    object->MoveTo(coords);
}

std::string Game::SerializeStatus(long int gold, int playerID) const {
    std::stringstream statusString;

    statusString << gold << '\n';
    for(auto& obj : objects) {
        statusString << obj->Serialize(playerID);
    }
    return statusString.str();
}

std::vector<const GameObject*> Game::GetObjectsAtPos(Coords pos) const {
    std::vector<const GameObject*> objectsAtPos;
    for(auto& o : objects) {
        auto oPos = o->GetPos();
        if(oPos == pos) {
            objectsAtPos.push_back(o.get());
        }
    }
    return objectsAtPos;
}

std::vector<const Unit *> Game::GetPlayerUnits(int playerID) const {
    std::vector<const Unit *> units;
    for(auto& o : objects) {
        if(o->GetPlayerID() == playerID && o->GetType() != ObjectType::BASE) {
            units.push_back(dynamic_cast<Unit*>(o.get()));
        }
    }
    return units;
}

std::vector<const GameObject *> Game::GetPlayerObjects(int playerID) const {
    std::vector<const GameObject *> playerObjects;
    for(auto& o : objects) {
        if(o->GetPlayerID() == playerID) {
            playerObjects.push_back(o.get());
        }
    }
    return playerObjects;
}

bool Game::IsValidPlacement(const Unit *unit, Coords pos) const {
    if(!map.IsValidPlacement(pos))
        return false;
    int playerID = unit->GetPlayerID();
    for(auto& o : objects) {
        if(o->GetPos() == pos) {
            if(o->GetPlayerID() == playerID)
                return true;
            else
                return false;
        }
    }
    return true;
}

int Game::CountObjectsForPlayer(int playerID) const {
    int count = 0;
    for(auto& o : objects) {
        count += o->GetPlayerID() == playerID ? 1 : 0;
    }
    return count;
}

int Game::CountObjects() const {
    return objects.size();
}

void Game::PayForUnit(int amount, int playerID) {
    long int& playerGold = playerID == 1 ? playerOneGold : playerTwoGold;
    if(playerGold < amount)
        throw std::invalid_argument("Player doesn't have enough gold.");
    playerGold -= amount;
}

int Game::GetTurnNumber() const {
    return turnNumber;
}

int Game::GetPlayerGold(int playerID) const {
    return playerID == 1 ? playerOneGold : playerTwoGold;
}

const GameMap &Game::GetMap() const {
    return map;
}

GameObject* Game::GetObjectWithID(int id) {
    for(auto& o : objects) {
        if(o->GetID() == id) {
            return o.get();
        }
    }
    return nullptr;
}

const Base *Game::GetPlayerBase(int playerID) const {
    for(auto& o : objects) {
        if(o->GetPlayerID() == playerID && o->GetType() == ObjectType::BASE) {
            return dynamic_cast<Base*>(o.get());
        }
    }
    return nullptr;
}

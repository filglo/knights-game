#include "Player.h"

#include "commands/Commands.h"
#include <iostream>
#include <fstream>
#include <queue>
#include <chrono>

enum class UnitState
{
    DEFEND_BASE,
    ATTACK_BASE,
    DEFEND_WORKER,
    ATTACK_WORKER,
    ATTACK_UNIT,
    DEFEND_UNIT
};

Player::Player(const char *programFilename, const char *mapFilename, const char *statusFilename, const char *commandsFilename, int timeLimit)
    : timeLimit(timeLimit)
    , mapFilename(mapFilename)
    , statusFilename(statusFilename)
    , commandsFilename(commandsFilename)
    , programFilename(programFilename)
    , game(mapFilename, statusFilename, statusFilename)
{
}

void Player::Run() {
    // Order units
    auto startTime = std::chrono::steady_clock::now();
    for(auto u : game.GetPlayerUnits(1)) {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime);
        // Leave some time for file io
        if(duration.count() >= timeLimit*1000 - 500)
            break;
        if(!CheckSurroundings(u)) {
            auto path = FindPath(u->GetPos(), game.GetPlayerBase(2)->GetPos(), GameConstants::GetUnitBaseMoves(u->GetType()), [](Coords c){return 0.f;});
            if(!path.empty())
                commands.AddMoveCommand(u->GetID(), path.back())->Execute(game);
        }
    }
    
    // Build units
    auto playerBase = game.GetPlayerBase(1);
    if(playerBase && !playerBase->IsBuilding() && game.GetPlayerGold(1) >= GameConstants::GetUnitPrice(ObjectType::ARCHER))
        commands.AddBuildCommand(playerBase->GetID(), ObjectType::ARCHER);

    // Save commands
    std::ofstream commandsFile(commandsFilename);
    commandsFile << commands.Serialize();
    commandsFile.close();
}

std::vector<Coords> Player::FindPath(Coords start, Coords destination, int moveDistance, std::function<float(Coords pos)> costFunction)
{
    // TODO: Refactor this!
    auto map = game.GetMap();
    int width = map.GetDimensions().first;
    int height = map.GetDimensions().second;
    std::vector<std::pair<float, int>> costs(width*height, {0.f, -1});
    auto greater = [](std::pair<float, int> a, std::pair<float, int> b){return a.first > b.first;};
    std::priority_queue<std::pair<float, int>, std::vector<std::pair<float, int>>, decltype(greater)> nodesToCheck(greater);
    bool foundDestination = false;
    auto startIdx = map.ConvertCoordsToIdx(start);
    costs[startIdx].second = startIdx;
    nodesToCheck.push({0.f, startIdx});
    while(!foundDestination && nodesToCheck.size() > 0) {
        auto currentNode = nodesToCheck.top();
        nodesToCheck.pop();
        auto c = map.ConvertIdxToCoords(currentNode.second);
        for(int i = -moveDistance; i <= moveDistance; i++) {
            if(!foundDestination) {
                for(int j = std::abs(i)-moveDistance; j <= moveDistance - std::abs(i); j++) {
                    int idxToCheck = map.ConvertCoordsToIdx({c.first+i, c.second+j});
                    if(std::make_pair(c.first+i, c.second+j) == destination) {
                        foundDestination = true;
                        costs[idxToCheck].second = currentNode.second;
                        break;
                    }
                    if(map.IsValidPlacement({c.first+i, c.second+j})) {

                        float cost = GameMap::Distance(c, {c.first+i, c.second+j})
                                   + costs[currentNode.second].first + costFunction({c.first+i, c.second+j}) + 0.1f;
                        float heur = GameMap::Distance({c.first+i, c.second+j}, destination);
                        
                        if(cost < costs[idxToCheck].first || costs[idxToCheck].second == -1) {
                            nodesToCheck.push({cost+heur, idxToCheck});
                            costs[idxToCheck].second = currentNode.second;
                            costs[idxToCheck].first = cost;
                        }
                    }
                }
            }
        }
    }
    std::vector<Coords> path;
    if(foundDestination) {
        int lastIdx = map.ConvertCoordsToIdx(destination);
        while(lastIdx != startIdx) {
            path.push_back(map.ConvertIdxToCoords(lastIdx));
            lastIdx = costs[lastIdx].second;
        }
    }
    return path;
}

bool Player::CheckSurroundings(const Unit *unit)
{
    const GameObject* closestEnemy = nullptr;
    int distanceToClosestEnemy = GameConstants::GetUnitBaseMoves(unit->GetType())+1;
    for(auto u : game.GetPlayerObjects(2)) {
        if(GameMap::Distance(*u, unit->GetPos()) <= distanceToClosestEnemy) {
            closestEnemy = u;
        }
    }
    if(closestEnemy == nullptr)
        return false;
    auto avoid = [](Coords c, Coords target) {
        return -2.f*GameMap::Distance(c, target);
    };
    auto engage = [unit](Coords c, Coords target) {
        return GameMap::Distance(c, target) <= GameConstants::GetUnitRange(unit->GetType()) ? -10.f : 0.f;
    };
    if(unit->HasAttacked()) {
        MoveUnit(unit, closestEnemy->GetPos(), avoid);
    } else {
        MoveUnit(unit, closestEnemy->GetPos(), engage);
        if(distanceToClosestEnemy <= GameConstants::GetUnitRange(unit->GetType())) {
            commands.AddAttackCommand(unit->GetID(), closestEnemy->GetID())->Execute(game);
            MoveUnit(unit, closestEnemy->GetPos(), avoid);
        }
    }
    return true;
}

void Player::MoveUnit(const Unit *unit, Coords destination, std::function<float(Coords pos, Coords target)> costFunction)
{
    int remainingMoves = unit->GetMoves();
    float minimumCost = 100;
    Coords bestPos = unit->GetPos();
    for(int i = -remainingMoves; i <= remainingMoves; i++) {
        for(int j = std::abs(i)-remainingMoves; j <= remainingMoves - std::abs(i); j++) {
            Coords pos = {0, 0};
            if(game.IsValidPlacement(unit, pos)) {
                float cost = GameMap::Distance(*unit, pos) + costFunction(pos, destination);
                if(cost <= minimumCost) {
                    minimumCost = cost;
                    bestPos = pos;
                }
            }
        }
    }
    commands.AddMoveCommand(unit->GetID(), bestPos)->Execute(game);
}

#include "Player.h"

#include "commands/Commands.h"
#include <iostream>
#include <fstream>
#include <queue>

#define DEBUG_OUTPUT 0

Player::Player(const char *programFilename, const char *mapFilename, const char *statusFilename, const char *commandsFilename, int timeLimit)
    : timeLimit(timeLimit)
    , mapFilename(mapFilename)
    , statusFilename(statusFilename)
    , commandsFilename(commandsFilename)
    , programFilename(programFilename)
    , game(mapFilename, statusFilename, statusFilename)
    , unitState(UnitState::ATTACK_BASE)
    , startTime(std::chrono::steady_clock::now())
{
    auto dims = game.GetMap().GetDimensions();
    workerHeatmap.resize(dims.first*dims.second, 0.f);
    unitHeatmap.resize(dims.first*dims.second, 0.f);
    playerBase = game.GetPlayerBase(1);
    enemyBase = game.GetPlayerBase(2);
}

void Player::Run() {
    if(!playerBase || !enemyBase)
        return;
    GenerateHeatmaps();
    GiveOrdersToUnits();
    BuildUnits();
    
    // Save commands
    std::ofstream commandsFile(commandsFilename);
    commandsFile << commands.Serialize();
    commandsFile.close();
}

void Player::BuildUnits() {
    if(!playerBase || !enemyBase || Timeout() || playerBase->IsBuilding())
        return;
    std::vector<std::pair<int, ObjectType>> units = {{5, ObjectType::ARCHER}, {3, ObjectType::CATAPULT}, {0, ObjectType::KNIGHT},
                                                       {1, ObjectType::WORKER}, {0, ObjectType::PIKEMAN}, {-1, ObjectType::SWORDSMAN}};
    auto updateUnits = [&units](ObjectType type) {
        switch (type)
        {
            case ObjectType::RAM:
                units[0].first += -1;
                units[1].first += 2;
                units[2].first += 6;
                units[4].first += -1;
                break;
            case ObjectType::ARCHER:
                units[1].first += 5;
                units[2].first += -1;
                units[4].first += -3;
                break;
            case ObjectType::WORKER:
                units[0].first += 2;
                units[2].first += 2;
                units[3].first += 1;
                units[4].first += 1;
                break;
            case ObjectType::CATAPULT:
                units[0].first += -6;
                units[1].first += 3;
                units[2].first += 3;
                units[4].first += -3;
                break;
            case ObjectType::KNIGHT:
                units[0].first += 1;
                units[2].first += -2;
                units[3].first += -5;
                units[4].first += 8;
                break;
            case ObjectType::PIKEMAN:
                units[0].first += 2;
                units[1].first += 1;
                units[2].first += -6;
                units[3].first += -2;
                break;
            default:
                break;
        }
    };
    if(game.GetPlayerGold(1) > 3000)
        units[3].first += -6;
    if(game.GetPlayerGold(1) > 2000)
        units[1].first += 2;
    if(game.GetPlayerGold(1) < 1000)
        units[3].first += 1;
    units[3].first += int(GameMap::Distance(playerBase->GetPos(), enemyBase->GetPos())/7);
    if(game.GetMap().GetMinePositions().empty())
        units[3].first += -1000;

    if(enemyBase->IsBuilding())
        updateUnits(enemyBase->GetUnitBuildType());

    for(auto u : game.GetPlayerUnits(2))
        updateUnits(u->GetType());

    int workerCount = 0;
    for(auto u : game.GetPlayerUnits(1))
        if(u->GetType() == ObjectType::WORKER)
            workerCount++;
    units[3].first += 2 - workerCount;
    
    std::sort(units.begin(), units.end(), [](auto p1, auto p2) { return p1.first > p2.first; });
    for(auto p : units) {
        if(game.GetPlayerGold(1) >= GameConstants::GetUnitPrice(p.second)) {
            commands.AddBuildCommand(playerBase->GetID(), p.second);
            break;
        }
    }
}

void Player::GiveOrdersToUnits() {
    for(auto u : game.GetPlayerUnits(1)) {
        if(Timeout())
            break;
        if(!CheckSurroundings(u)) {
            std::vector<Coords> path;
            if(u->GetType() == ObjectType::WORKER) {
                int minDist = 2000;
                Coords bestMine = enemyBase->GetPos();
                for(auto mPos : game.GetMap().GetMinePositions()) {
                    if(GameMap::Distance(u->GetPos(), mPos) < minDist) {
                        minDist = GameMap::Distance(u->GetPos(), mPos);
                        bestMine = mPos;
                    }
                }
                auto costFunction = [this](int idx){return this->workerHeatmap[idx];};
                path = FindPath(u->GetPos(), bestMine, GameConstants::GetUnitBaseMoves(u->GetType()), costFunction);
            } else {
                Coords target = unitState == UnitState::ATTACK_BASE ? enemyBase->GetPos() : playerBase->GetPos();
                auto costFunction = [this](int idx){return this->unitHeatmap[idx];};
                path = FindPath(u->GetPos(), target, GameConstants::GetUnitBaseMoves(u->GetType()), costFunction);
            }
            if(!path.empty() && path.back() != u->GetPos())
                TryToExecuteCommand(commands.AddMoveCommand(u->GetID(), path.back()));
        }
    }
}

void Player::GenerateHeatmaps() {
    if(!playerBase || !enemyBase)
        return;
    unitState = UnitState::ATTACK_BASE;
    auto map = game.GetMap();
    for(auto mPos : map.GetMinePositions()) {
        if(Timeout()) return;
        UpdateHeatmap(workerHeatmap, -5.0f, 5, mPos);
    }
    for(auto u : game.GetPlayerObjects(1)) {
        if(Timeout()) return;
        if(u->GetType() == ObjectType::BASE) {
            UpdateHeatmap(unitHeatmap, -5.f, 7, u->GetPos());
            UpdateHeatmap(workerHeatmap, -2.5f, 7, u->GetPos());
        }
        else {
            int friendRangeRadius = GameConstants::GetUnitBaseMoves(u->GetType());
            UpdateHeatmap(unitHeatmap, -GameConstants::GetUnitPrice(u->GetType()) / 50.f, friendRangeRadius, u->GetPos());
            UpdateHeatmap(workerHeatmap, -2.5f, friendRangeRadius, u->GetPos());
        }
    }
    for(auto u : game.GetPlayerObjects(2)) {
        if(Timeout()) return;
        if(u->GetType() == ObjectType::BASE) {
            UpdateHeatmap(unitHeatmap, -3.f, 6, u->GetPos());
        } else {
            UpdateHeatmap(workerHeatmap, 2.5f, 6, u->GetPos());
            auto d = GameMap::Distance(*u, playerBase->GetPos());
            float value = d - 20.f;
            if(u->GetType() == ObjectType::WORKER)
                value += -3.f;
            else if(u->GetType() == ObjectType::RAM)
                value += -3.f;
            else
                value += GameConstants::GetUnitPrice(u->GetType()) / 50.f;
            int enemyRangeRadius = GameConstants::GetUnitRange(u->GetType()) + GameConstants::GetUnitBaseMoves(u->GetType()) - 1;
            UpdateHeatmap(unitHeatmap, value, enemyRangeRadius, u->GetPos());
            if(d < std::min(std::min(15, map.GetDimensions().first), map.GetDimensions().second))
                unitState = UnitState::DEFEND_BASE;
        }
    }
}

void Player::UpdateHeatmap(std::vector<float>& heatmap, float value, int distance, Coords pos) {
    for(int i = -distance; i <= distance; i++) {
        for(int j = std::abs(i)-distance; j <= distance - std::abs(i); j++) {
            Coords c = {pos.first + i, pos.second + j};
            if(game.GetMap().IsInBounds(c)) {
                int idx = game.GetMap().ConvertCoordsToIdx(c);
                heatmap[idx] += value;
            }
        }
    }
}

void Player::TryToExecuteCommand(std::shared_ptr<Command> command)
{
    try {
        command->Execute(game);
    } catch(const std::logic_error& le) {
        if(DEBUG_OUTPUT) std::cout << "Failed command: " << le.what() << '\n';
        commands.PopLastCommand();
    }
}

std::vector<Coords> Player::FindPath(Coords start, Coords destination, int moveDistance, std::function<float(int idx)> costFunction)
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
    while(!foundDestination && nodesToCheck.size() > 0 && !Timeout()) {
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
                                   + costs[currentNode.second].first + std::max(0.1f, costFunction(idxToCheck) + 4.f);
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
        int loopIdx = 0;
        while(lastIdx != startIdx && loopIdx < 1000) {
            loopIdx++;
            path.push_back(map.ConvertIdxToCoords(lastIdx));
            lastIdx = costs[lastIdx].second;
        }
    }
    return path;
}

bool Player::CheckSurroundings(const Unit *unit)
{
    const GameObject* closestEnemy = nullptr;
    int distanceToClosestEnemy = unit->GetMoves() - 1 + GameConstants::GetUnitRange(unit->GetType());
    if(distanceToClosestEnemy <= 0)
        return true;
    for(auto u : game.GetPlayerObjects(2)) {
        if(Timeout()) return false;
        if(!u->IsDestroyed() && GameMap::Distance(*u, unit->GetPos()) <= distanceToClosestEnemy) {
            closestEnemy = u;
            distanceToClosestEnemy = GameMap::Distance(*u, unit->GetPos());
        }
    }
    if(closestEnemy == nullptr)
        return false;

    auto map = game.GetMap();
    const std::vector<float>& heatmap = unitHeatmap;
    auto avoid = [map, heatmap](Coords c, Coords target) {
        return -2.f*GameMap::Distance(c, target) + heatmap[map.ConvertCoordsToIdx(c)];
    };
    auto engage = [unit, map, heatmap](Coords c, Coords target) {
        return (GameMap::Distance(c, target) <= GameConstants::GetUnitRange(unit->GetType()) ? -10.f : 0.f) + heatmap[map.ConvertCoordsToIdx(c)];
    };
    if(unit->HasAttacked()) {
        MoveUnit(unit, closestEnemy->GetPos(), avoid);
    } else {
        if(distanceToClosestEnemy <= GameConstants::GetUnitRange(unit->GetType())) {
            TryToExecuteCommand(commands.AddAttackCommand(unit->GetID(), closestEnemy->GetID()));
            MoveUnit(unit, closestEnemy->GetPos(), avoid);
        } else {
            MoveUnit(unit, closestEnemy->GetPos(), engage);
            if(distanceToClosestEnemy <= GameConstants::GetUnitRange(unit->GetType())) {
                TryToExecuteCommand(commands.AddAttackCommand(unit->GetID(), closestEnemy->GetID()));
            }
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
            Coords pos = unit->GetPos();
            pos.first += i;
            pos.second += j;
            if(game.IsValidPlacement(unit, pos)) {
                float cost = GameMap::Distance(*unit, pos) + costFunction(pos, destination);
                if(cost <= minimumCost) {
                    minimumCost = cost;
                    bestPos = pos;
                }
            }
        }
    }
    TryToExecuteCommand(commands.AddMoveCommand(unit->GetID(), bestPos));
}

bool Player::Timeout() const
{
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime);
    // Leaving time for file io and finishing program execution
    if(duration.count() >= timeLimit*1000 - 500)
        return false;
    return false;
}

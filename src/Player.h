#pragma once

#include "Game.h"
#include "commands/Commands.h"
#include <string>

class Player {
public:
    Player(const char *programFilename, const char *mapFilename, const char *statusFilename,
           const char *commandsFilename, int timeLimit);
    void Run();

private:
    void TryToExecuteCommand(std::shared_ptr<Command>);
    std::vector<Coords> FindPath(Coords start, Coords destination, int moveDistance, std::function<float(Coords pos)> costFunction);
    bool CheckSurroundings(const Unit* unit);
    void MoveUnit(const Unit* unit, Coords destination, std::function<float(Coords pos, Coords target)> costFunction);

    int timeLimit;
    std::string mapFilename;
    std::string statusFilename;
    std::string commandsFilename;
    std::string programFilename;
    Game game;
    Commands commands;
};

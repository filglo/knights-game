#pragma once

#include "Game.h"
#include <string>

class Mediator {
public:
    Mediator(const char *mapFilename, const char *p1Filename, const char *p2Filename, const char *statusP1Filename,
             const char *statusP2Filename, const char *commandsP1Filename, const char *commandsP2Filename, int timeLimit);
    void Run();

private:
    GameWinState RunProgram(const char* program, const char* map, const char* status, const char* commands, int timeLimit) const;
    bool ProcessGameWinState(GameWinState gameWinState) const;
    int GetCurrentPlayerID() const;

    int timeLimit;
    std::string mapFilename;
    std::string statusP1Filename;
    std::string statusP2Filename;
    std::string commandsP1Filename;
    std::string commandsP2Filename;
    std::string programP1Filename;
    std::string programP2Filename;
    Game game;
};
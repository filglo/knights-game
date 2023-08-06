#include "Mediator.h"

#include "commands/Commands.h"
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fstream>
#include <chrono>
#include <string.h>

Mediator::Mediator(const char* mapFilename, const char* p1Filename, const char* p2Filename,
                   const char* statusP1Filename, const char* statusP2Filename, const char* commandsP1Filename,
                   const char* commandsP2Filename, int timeLimit)
    : timeLimit(timeLimit)
    , mapFilename(mapFilename)
    , statusP1Filename(statusP1Filename)
    , statusP2Filename(statusP2Filename)
    , commandsP1Filename(commandsP1Filename)
    , commandsP2Filename(commandsP2Filename)
    , programP1Filename(p1Filename)
    , programP2Filename(p2Filename)
    , game(mapFilename, statusP1Filename, statusP2Filename)
{
}

void Mediator::Run() {
    Commands commands;
    for(int turn = 0; turn < 2000; turn++) {
        auto playerID = GetCurrentPlayerID();
        const char* programFilename = playerID == 1 ? programP1Filename.c_str() : programP2Filename.c_str();
        const char* statusFilename = playerID == 1 ? statusP1Filename.c_str() : statusP2Filename.c_str();
        const char* commandsFilename = playerID == 1 ? commandsP1Filename.c_str() : commandsP2Filename.c_str();
        auto gameWinState = RunProgram(programFilename, mapFilename.c_str(), statusFilename, commandsFilename, timeLimit);
        if(ProcessGameWinState(gameWinState))
            return;

        try {
            commands.Deserialize(commandsFilename);
            commands.ExecuteCommands(game);
        } catch(const std::invalid_argument& ia) {
	        std::cerr << "Invalid argument: " << ia.what() << '\n';
            ProcessGameWinState(playerID == 1 ? GameWinState::PLAYER1_ERROR : GameWinState::PLAYER2_ERROR);
            return;
        } catch(const std::logic_error& le) {
	        std::cerr << "Logic error: " << le.what() << '\n';
            ProcessGameWinState(playerID == 1 ? GameWinState::PLAYER1_ERROR : GameWinState::PLAYER2_ERROR);
            return;
        } catch(...) {
	        std::cerr << "Unhandled exception" << '\n';
            ProcessGameWinState(playerID == 1 ? GameWinState::PLAYER1_ERROR : GameWinState::PLAYER2_ERROR);
            throw;
        }

        if(ProcessGameWinState(game.Turn()))
            return;

        playerID = GetCurrentPlayerID();
        statusFilename = playerID == 1 ? statusP1Filename.c_str() : statusP2Filename.c_str();
        auto statusString = game.SerializeStatus(game.GetPlayerGold(playerID), playerID);
        std::ofstream statusFile(statusFilename);
        statusFile << statusString;
        statusFile.close();
    }
}

GameWinState Mediator::RunProgram(const char* program, const char* map, const char* status, const char* commands, int timeLimit) const {
    int st;
    auto startTime = std::chrono::steady_clock::now();
    int pid = fork();
    std::string timeStr = std::to_string(timeLimit);
    if(pid == 0)
        if(execl(program, program, map, status, commands, timeStr.c_str(), (char*)0)) {
            char buffer[256];
            char* errorMsg = strerror_r(errno, buffer, 256);
            std::cerr << errorMsg << std::endl;
            _exit(EXIT_FAILURE);
        }
    while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count() <= timeLimit*1000)
    {
        int r = waitpid(pid, &st, WNOHANG);
        if(r == -1 || (r > 0 && !WIFEXITED(st))) {
            return GetCurrentPlayerID() == 1 ? GameWinState::PLAYER1_ERROR : GameWinState::PLAYER2_ERROR;
        } else if(r > 0 && WIFEXITED(st)) {
            if(WEXITSTATUS(st) == EXIT_FAILURE)
                return GetCurrentPlayerID() == 1 ? GameWinState::PLAYER1_ERROR : GameWinState::PLAYER2_ERROR;
            return GameWinState::IN_PROGRESS;
        }
    }
    return GetCurrentPlayerID() == 1 ? GameWinState::PLAYER1_TIMEOUT : GameWinState::PLAYER2_TIMEOUT;
}

bool Mediator::ProcessGameWinState(GameWinState gameWinState) const {
    switch(gameWinState)
    {
        case GameWinState::PLAYER1_WIN:
            std::cout << "Player 1 wins on turn ";
            break;
        case GameWinState::PLAYER2_WIN:
            std::cout << "Player 2 wins on turn ";
            break;
        case GameWinState::PLAYER1_ERROR:
            std::cout << "Player 1 error on turn ";
            break;
        case GameWinState::PLAYER2_ERROR:
            std::cout << "Player 2 error on turn ";
            break;
        case GameWinState::PLAYER1_TIMEOUT:
            std::cout << "Player 1 timeouts on turn ";
            break;
        case GameWinState::PLAYER2_TIMEOUT:
            std::cout << "Player 2 timeouts on turn ";
            break;
        case GameWinState::TIE:
            std::cout << "Tie on turn ";
            break;
        default:
            return false;
    }
    std::cout << game.GetTurnNumber()+1 << std::endl;
    return true;
}

int Mediator::GetCurrentPlayerID() const {
    return game.GetTurnNumber() % 2 + 1;
}


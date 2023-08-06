#pragma once

#include "Command.h"
#include "../Game.h"
#include <vector>
#include <string>
#include <memory>

class Commands {
public:
    Commands();

    void Deserialize(const char* commandsFilename);
    std::string Serialize() const;
    void ExecuteCommands(Game& game);
    void PopLastCommand();
    std::shared_ptr<Command> AddAttackCommand(int unit, int attackedUnit);
    std::shared_ptr<Command> AddBuildCommand(int unit, ObjectType object);
    std::shared_ptr<Command> AddMoveCommand(int unit, Coords c);

private:
    std::vector<std::shared_ptr<Command>> commands;
};


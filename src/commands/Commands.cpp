#include "Commands.h"

#include "Attack.h"
#include "Build.h"
#include "Move.h"
#include <fstream>
#include <sstream>
#include <exception>

Commands::Commands() {

}

void Commands::Deserialize(const char* commandsFilename) {
    std::ifstream commandsFileS(commandsFilename);
    int unitID;
    char identifier;

    while(commandsFileS >> unitID >> identifier) {
        switch (identifier)
        {
            case 'M':
                int posX, posY;
                commandsFileS >> posX >> posY;
                AddMoveCommand(unitID, {posX, posY});
                break;
            case 'A':
                int id;
                commandsFileS >> id;
                AddAttackCommand(unitID, id);
                break;
            case 'B':
                char buildIdentifier;
                commandsFileS >> buildIdentifier;
                AddBuildCommand(unitID, GameConstants::GetTypeFromIdentifier(buildIdentifier));
                break;
            default:
                throw std::invalid_argument("Invalid command token.");
                break;
        }
    }
    commandsFileS.close();
}

std::string Commands::Serialize() const {
    std::stringstream commandString;

    for(auto command : commands) {
        commandString << command->Serialize();
    }
    return commandString.str();
}

std::shared_ptr<Command> Commands::AddAttackCommand(int unit, int attackedUnit) {
    commands.emplace_back(std::make_shared<Attack>(unit, attackedUnit));
    return commands.back();
}

std::shared_ptr<Command> Commands::AddBuildCommand(int unit, ObjectType object) {
    commands.emplace_back(std::make_shared<Build>(unit, object));
    return commands.back();
}

std::shared_ptr<Command> Commands::AddMoveCommand(int unit, Coords c) {
    commands.emplace_back(std::make_shared<Move>(unit, c));
    return commands.back();
}

void Commands::ExecuteCommands(Game& game) {
    for(auto command : commands) {
        command->Execute(game);
    }
    commands.clear();
}

void Commands::PopLastCommand() {
    if(!commands.empty())
        commands.pop_back();
}

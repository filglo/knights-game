#pragma once

#include <string>
#include "../Game.h"

class Command {
public:
    virtual ~Command() = default;
    virtual std::string Serialize() = 0;
    virtual void Execute(Game& game) = 0;
};
#include "Player.h"

#include <string>

int main(int argc, char *argv[])
{
    // Order of arguments:
    // 0. Program name
    // 1. Map filename
    // 2. Status filename
    // 3. Commands filename
    // 4. Time limit in seconds (default = 5)
    int timeLimit = argc > 4 ? std::stoi(argv[4]) : 5;
    Player player(argv[0], argv[1], argv[2], argv[3], timeLimit);
    player.Run();
    return 0;
}

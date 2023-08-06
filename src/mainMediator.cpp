#include "Mediator.h"

#include <string>

int main(int argc, char *argv[])
{
    // Order of arguments:
    // 1. Map filename
    // 2. Program of player 1
    // 3. Program of player 2
    // 4. Status filename of player 1
    // 5. Status filename of player 2
    // 6. Commands filename of player 1
    // 7. Commands filename of player 2
    // 8. Time limit in seconds
    Mediator mediator(argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], std::stoi(argv[8]));
    mediator.Run();
    return 0;
}

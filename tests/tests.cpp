#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "GameMap.h"
#include "GameObject.h"
#include "Game.h"
#include "commands/Attack.h"
#include "commands/Move.h"
#include "commands/Build.h"
#include "commands/Commands.h"

TEST_CASE("Testing map serialization") {
    bool exceptionThrown = false;
    try {
        GameMap map("tests/mapTest.txt");
    } catch(const std::invalid_argument& ia) {
        exceptionThrown = true;
    }
    CHECK(exceptionThrown == false);
}

TEST_CASE("Testing map const methods") {
    GameMap map("tests/mapTest.txt");
    
    CHECK(GameMap::Distance({1,1}, {2,-2}) == 4);
    CHECK(GameMap::Distance({0,0}, {0, 0}) == 0);
    CHECK(map.GetDimensions() == std::pair<int, int>(6, 4));
    CHECK(map.ConvertCoordsToIdx({3,2}) == 15);
    CHECK(map.ConvertCoordsToIdx({0,0}) == 0);
    CHECK(map.ConvertCoordsToIdx({0,1}) == 6);
    CHECK(map.ConvertIdxToCoords(15) == Coords(3, 2));
    CHECK(map.ConvertIdxToCoords(0) == Coords(0, 0));
    CHECK(map.ConvertIdxToCoords(6) == Coords(0, 1));
    CHECK(map.ConvertIdxToCoords(23) == Coords(5, 3));
    CHECK(map.IsInBounds({2,3}) == true);
    CHECK(map.IsInBounds({2,-1}) == false);
    CHECK(map.IsInBounds({0,4}) == false);
    CHECK(map.IsValidPlacement({0,0}) == true);
    CHECK(map.IsValidPlacement({4,1}) == false);
    CHECK(map.IsValidPlacement({1,3}) == true);
}

TEST_CASE("Testing mine positions") {
    GameMap map("tests/mapTest.txt");

    auto minePositions = map.GetMinePositions();
    REQUIRE(minePositions.size() == 1);
    SUBCASE("") {
        CHECK(minePositions[0] == Coords(1, 3));
    }
    CHECK(map.GetTileAtPos({1,3}) == MapTile::MINE);
}

TEST_CASE("Testing Archer const methods") {
    auto archer = Unit(ObjectType::ARCHER, 0, {6, 7}, 2, 40);
    CHECK(archer.GetDurability() == 40);
    CHECK(archer.GetID() == 0);
    CHECK(archer.GetMoves() == 2);
    CHECK(archer.GetPlayerID() == 2);
    CHECK(archer.GetPos() == Coords(6, 7));
    CHECK(archer.GetType() == ObjectType::ARCHER);
    CHECK(archer.HasAttacked() == false);
    CHECK(archer.IsDestroyed() == false);
}

TEST_CASE("Testing Knight serialize") {
    auto knight = Unit(ObjectType::KNIGHT, 0, {2, 3}, 2, 50);
    CHECK(knight.Serialize(1).compare("E K 0 2 3 50\n") == 0);
}

TEST_CASE("Testing Knight actions") {
    auto knight = Unit(ObjectType::KNIGHT, 0, {2, 3}, 2, 20);
    SUBCASE("Knight attacks") {
        knight.Attack();
        REQUIRE(knight.GetMoves() == 4);
        REQUIRE(knight.HasAttacked() == true);
        SUBCASE("Knight end of turn") {
            knight.TakeTurn();
            CHECK(knight.GetMoves() == 5);
            CHECK(knight.HasAttacked() == false);
        }
    }
    SUBCASE("Knight gets damaged") {
        knight.Damage(10);
        CHECK(knight.IsDestroyed() == false);
        CHECK(knight.GetDurability() == 10);
    }
    SUBCASE("Knight gets destroyed") {
        bool test = false;
        auto f = [&test](int a, int b) { test = true; };
        knight.AddCallbackOnDestroy(f);
        knight.Damage(25);
        CHECK(knight.IsDestroyed() == true);
        CHECK(test == true);
    }
    SUBCASE("Knight moves") {
        knight.MoveTo({3, 4});
        CHECK(knight.GetMoves() == 3);
    }
    SUBCASE("Knight moves again") {
        knight.MoveTo({-2, 4});
        CHECK(knight.GetMoves() == 0);
    }
    SUBCASE("Assert exception when moving out of range") {
        bool exceptionThrown = false;
        try {
            knight.MoveTo({7,2});
        } catch(const std::logic_error& le) {
            exceptionThrown = true;
        }
        CHECK(exceptionThrown == true);
    }
}

TEST_CASE("Testing Base basic methods") {
    auto base = Base(5, {1, 2}, 1, 200, ObjectType::CATAPULT, false);
    CHECK(base.GetPlayerID() == 1);
    CHECK(base.GetID() == 5);
    CHECK(base.IsBuilding() == false);
}

TEST_CASE("Testing Base serialization 1") {
    auto base = Base(5, {1, 2}, 1, 200, ObjectType::CATAPULT, false);
    CHECK(base.Serialize(1).compare("P B 5 1 2 200 0\n") == 0);
}

TEST_CASE("Testing Base serialization 2") {
    auto base = Base(0, {1, 6}, 1, 200, ObjectType::CATAPULT, true);
    CHECK(base.Serialize(1).compare("P B 0 1 6 200 C\n") == 0);
}

TEST_CASE("Testing Base cannot move") {
    auto base = Base(5, {1, 2}, 1, 200, ObjectType::CATAPULT, false);
    bool exceptionThrown = false;
    try {
        base.MoveTo({10, 10});
    } catch(const std::logic_error& le) {
            exceptionThrown = true;
    }
    CHECK(exceptionThrown == true);
}

TEST_CASE("Testing Base build methods") {
    auto base = Base(5, {1, 2}, 1, 200, ObjectType::CATAPULT, false);
    SUBCASE("Testing base cannot build base") {
        bool exceptionThrown = false;
        try {
            base.Build(ObjectType::BASE);
        } catch(const std::invalid_argument& ia) {
                exceptionThrown = true;
        }
        CHECK(exceptionThrown == true);
    }
    SUBCASE("Testing Base building Worker") {
        base.Build(ObjectType::WORKER);
        bool buildCompleted = false;
        ObjectType typeTest = ObjectType::WORKER;
        Coords coordTest = Coords(1, 2);
        int playerTest = 1;
        auto f = [&buildCompleted, typeTest, coordTest, playerTest](ObjectType t, Coords c, int a) {
            buildCompleted = (t == typeTest && c == coordTest && a == playerTest);
        };
        base.AddCallbackOnBuild(f);
        REQUIRE(base.IsBuilding() == true);
        REQUIRE(base.GetTurnsToBuild() == 2);
        REQUIRE(base.GetUnitBuildType() == ObjectType::WORKER);
        SUBCASE("Testing Base building Worker next turn") {
            base.TakeTurn();
            REQUIRE(base.IsBuilding() == true);
            REQUIRE(base.GetTurnsToBuild() == 1);
            REQUIRE(base.GetUnitBuildType() == ObjectType::WORKER);
            SUBCASE("Testing Base completing Worker") {
                base.TakeTurn();
                CHECK(base.IsBuilding() == false);
                CHECK(buildCompleted);
            }
        }
    }
}

TEST_CASE("Testing Command serialization") {
    SUBCASE("Testing Attack") {
        CHECK(Attack(5, 6).Serialize().compare("5 A 6\n") == 0);
    }
    SUBCASE("Testing Move") {
        CHECK(Move(6, {1, 3}).Serialize().compare("6 M 1 3\n") == 0);
    }
    SUBCASE("Testing Build") {
        CHECK(Build(7, ObjectType::PIKEMAN).Serialize().compare("7 B P\n") == 0);
    }
    SUBCASE("Testing Commands") {
        Commands commands;
        commands.AddAttackCommand(5, 6);
        commands.AddMoveCommand(6, {1, 3});
        commands.AddBuildCommand(7, ObjectType::PIKEMAN);
        CHECK(commands.Serialize().compare("5 A 6\n6 M 1 3\n7 B P\n") == 0);
    }
}

TEST_CASE("Testing Commands Deserialization") {
    Commands commands;
    commands.Deserialize("tests/commandsTest.txt");
    CHECK(commands.Serialize().compare("2 M 0 1\n4 M 0 1\n1 A 8\n3 B A\n") == 0);
}

TEST_CASE("Testing Game 1") {
    Game game("tests/mapTest.txt", "tests/statusTest.txt", "tests/statusTest.txt");

    SUBCASE("Testing Game GetObjectsAtPos") {
        auto objects = game.GetObjectsAtPos({1, 1});
        REQUIRE(objects.size() == 2);
        REQUIRE(objects[0] != nullptr);
        REQUIRE(objects[1] != nullptr);
        CHECK(objects[0]->GetID() == 5);
        CHECK(objects[1]->GetID() == 6);
    }
    
    SUBCASE("Testing Game.Base") {
        auto base = game.GetPlayerBase(2);
        CHECK(base->GetPlayerID() == 2);
        CHECK(base->GetType() == ObjectType::BASE);
        CHECK(base->GetID() == 0);
        CHECK(base->GetPos() == Coords(0, 0));
    }

    SUBCASE("Testing Game GetPlayerUnits") {
        auto units = game.GetPlayerUnits(1);
        CHECK(units.size() == 2);
        REQUIRE(units[0] != nullptr);
        REQUIRE(units[1] != nullptr);
        bool test1 = (units[0]->GetID() == 5) && (units[1]->GetID() == 6);
        bool test2 = (units[0]->GetID() == 6) && (units[1]->GetID() == 5);
        CHECK((test1 || test2) == true);
        CHECK(units[0]->GetType() != ObjectType::BASE);
        CHECK(units[1]->GetType() != ObjectType::BASE);
    }

    SUBCASE("Testing Game GetPlayerObjects") {
        auto objects = game.GetPlayerObjects(2);
        CHECK(objects.size() == 2);
        REQUIRE(objects[0] != nullptr);
        REQUIRE(objects[1] != nullptr);
        bool test = objects[0]->GetID() == 2 || objects[1]->GetID() == 2;
        CHECK(test == true);
        test = objects[0]->GetType() == ObjectType::BASE || objects[1]->GetType() == ObjectType::BASE;
        CHECK(test == true);
    }
    
    SUBCASE("Testing Game getters") {
        auto units = game.GetPlayerUnits(2);
        REQUIRE(units.size() == 1);
        REQUIRE(units[0]);
        CHECK(game.IsValidPlacement(units[0], {3, 2}) == false);
        CHECK(game.IsValidPlacement(units[0], {1, 1}) == false);
        CHECK(game.IsValidPlacement(units[0], {0, 0}) == true);

        CHECK(game.CountObjectsForPlayer(2) == 2);
        CHECK(game.GetTurnNumber() == 0);
        CHECK(game.GetPlayerGold(1) == 1950);
        CHECK(game.GetPlayerGold(2) == 1950);
    }
}

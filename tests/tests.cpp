#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "GameMap.h"

TEST_CASE("Testing map") {
    GameMap map;
    map.Deserialize("../tests/mapTest.txt");
    
    SUBCASE("") {
        CHECK(GameMap::Distance({1,1}, {2,-2}) == 4);
        CHECK(GameMap::Distance({0,0}, {0, 0}) == 0);
        CHECK(map.GetDimensions() == std::pair<int, int>(6, 4));
        CHECK(map.ConvertCoordsToIdx({3,2}) == 16);
        CHECK(map.ConvertIdxToCoords(16) == std::pair<int, int>(3, 2));
        CHECK(map.IsInBounds({2,3}) == true);
        CHECK(map.IsInBounds({2,-1}) == false);
        CHECK(map.IsInBounds({0,4}) == false);
        CHECK(map.IsValidPlacement({0,0}) == true);
        CHECK(map.IsValidPlacement({4,1}) == false);
        CHECK(map.IsValidPlacement({1,3}) == true);
    }
    SUBCASE("") {
        auto minePositions = map.GetMinePositions();
        REQUIRE(minePositions.size() == 1);
        SUBCASE("") {
            CHECK(minePositions[0] == std::pair<int, int>(1, 3));
        }
        CHECK(map.GetTileAtPos({1,3}) == MapTile::MINE);
    }
}

TEST_CASE("Testing Archer") {

}

TEST_CASE("Testing Base") {

}

TEST_CASE("Testing Game getters") {
    
}
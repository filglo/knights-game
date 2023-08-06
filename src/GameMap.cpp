#include "GameMap.h"

#include <fstream>
#include <string>
#include <sstream>

GameMap::GameMap() {

}

void GameMap::Deserialize(const char* map) {
    std::ifstream mapFile(map);

    int x = 0;
    for(std::string line; getline(mapFile, line);) {
        for(auto c : line) {
            auto type = GameConstants::GetMapTileFromIdentifier(c);
            mapTiles.push_back(type);
            if(type == MapTile::MINE)
                minePositions.push_back({x, height});
            x++;
        }
        x = 0;
        height++;
    }
    width = mapTiles.size()/height;
    mapFile.close();
}

MapTile GameMap::GetTileAtPos(Coords pos) const {
    return mapTiles[pos.first+pos.second*height];
}

const std::vector<Coords>& GameMap::GetMinePositions() const {
    return minePositions;
}

std::pair<int, int> GameMap::GetDimensions() const {
    return {width, height};
}

int GameMap::ConvertCoordsToIdx(Coords pos) const
{
    return pos.first + pos.second*width;
}

Coords GameMap::ConvertIdxToCoords(int idx) const
{
    int x, y;
    x = idx % width;
    y = idx - x;
    return {x, y};
}

bool GameMap::IsInBounds(Coords pos) const
{
    return pos.first >= 0 && pos.first < width && pos.second >= 0 && pos.second < height;
}

bool GameMap::IsValidPlacement(Coords pos) const
{
    return IsInBounds(pos) && GetTileAtPos(pos) != MapTile::OBSTACLE;
}

int GameMap::Distance(Coords posA, Coords posB)
{
    return std::abs(posA.first-posB.first)+std::abs(posA.second-posB.second);
}

int GameMap::Distance(const GameObject& object, Coords pos) {
    auto posA = object.GetPos();
    return Distance(posA, pos);
}

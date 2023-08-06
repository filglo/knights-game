#pragma once

#include "GameObject.h"
#include "GameConstants.h"
#include <vector>
#include <string>
#include <memory>

class GameMap {

public:
    GameMap();

    void Deserialize(const char* map);
    MapTile GetTileAtPos(Coords pos) const;
    const std::vector<Coords>& GetMinePositions() const;
    std::pair<int, int> GetDimensions() const;
    int ConvertCoordsToIdx(Coords pos) const;
    Coords ConvertIdxToCoords(int idx) const;
    bool IsInBounds(Coords pos) const;
    bool IsValidPlacement(Coords pos) const;
    static int Distance(Coords posA, Coords posB);
    static int Distance(const GameObject& object, Coords pos);

private:
    int width, height;
    std::vector<MapTile> mapTiles;
    std::vector<Coords> minePositions;
    
};

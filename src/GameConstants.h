#pragma once

#include <unordered_map>

enum class ObjectType {BASE, KNIGHT, SWORDSMAN, ARCHER, PIKEMAN, RAM, CATAPULT, WORKER};
enum class MapTile {EMPTY, OBSTACLE, MINE, BASE_PLAYER, BASE_OPPONENT};

namespace GameConstants {
    ObjectType GetTypeFromIdentifier(char id);
    char GetIdentifierFromType(ObjectType type);
    MapTile GetMapTileFromIdentifier(char id);
    int GetUnitPrice(ObjectType type);
    int GetUnitBaseMoves(ObjectType type);
    int GetUnitBaseDurability(ObjectType type);
    int GetUnitTurnsToBuild(ObjectType type);
    int GetUnitRange(ObjectType type);
    int GetUnitDamageToTarget(ObjectType type, ObjectType target);
    inline const std::unordered_map<ObjectType, std::tuple<char, int, int, int, int, int, std::unordered_map<ObjectType, int>>> unitConfig = {
    {ObjectType::BASE, {'B', 200, 0, 0, 0, 0, {{ObjectType::BASE, 0}, {ObjectType::KNIGHT, 0}, {ObjectType::SWORDSMAN, 0}, {ObjectType::ARCHER, 0}, {ObjectType::PIKEMAN, 0},
        {ObjectType::RAM, 0}, {ObjectType::CATAPULT, 0}, {ObjectType::WORKER, 0}}}},
    {ObjectType::KNIGHT, {'K', 70, 5, 400, 5, 1, {{ObjectType::BASE, 35}, {ObjectType::KNIGHT, 35}, {ObjectType::SWORDSMAN, 35}, {ObjectType::ARCHER, 35}, {ObjectType::PIKEMAN, 35},
        {ObjectType::RAM, 35}, {ObjectType::CATAPULT, 50}, {ObjectType::WORKER, 35}}}},
    {ObjectType::SWORDSMAN, {'S', 60, 3, 250, 2, 1, {{ObjectType::BASE, 30}, {ObjectType::KNIGHT, 30}, {ObjectType::SWORDSMAN, 30}, {ObjectType::ARCHER, 30}, {ObjectType::PIKEMAN, 20},
        {ObjectType::RAM, 20}, {ObjectType::CATAPULT, 30}, {ObjectType::WORKER, 30}}}},
    {ObjectType::ARCHER, {'A', 40, 3, 250, 2, 5, {{ObjectType::BASE, 15}, {ObjectType::KNIGHT, 15}, {ObjectType::SWORDSMAN, 15}, {ObjectType::ARCHER, 15}, {ObjectType::PIKEMAN, 15},
        {ObjectType::RAM, 10}, {ObjectType::CATAPULT, 10}, {ObjectType::WORKER, 15}}}},
    {ObjectType::PIKEMAN, {'P', 50, 3, 200, 2, 2, {{ObjectType::BASE, 10}, {ObjectType::KNIGHT, 35}, {ObjectType::SWORDSMAN, 15}, {ObjectType::ARCHER, 15}, {ObjectType::PIKEMAN, 15},
        {ObjectType::RAM, 15}, {ObjectType::CATAPULT, 10}, {ObjectType::WORKER, 15}}}},
    {ObjectType::RAM, {'R', 90, 4, 500, 2, 1, {{ObjectType::BASE, 50}, {ObjectType::KNIGHT, 10}, {ObjectType::SWORDSMAN, 10}, {ObjectType::ARCHER, 10}, {ObjectType::PIKEMAN, 10},
        {ObjectType::RAM, 10}, {ObjectType::CATAPULT, 10}, {ObjectType::WORKER, 10}}}},
    {ObjectType::CATAPULT, {'C', 50, 6, 800, 2, 7, {{ObjectType::BASE, 50}, {ObjectType::KNIGHT, 40}, {ObjectType::SWORDSMAN, 40}, {ObjectType::ARCHER, 40}, {ObjectType::PIKEMAN, 10},
        {ObjectType::RAM, 40}, {ObjectType::CATAPULT, 40}, {ObjectType::WORKER, 40}}}},
    {ObjectType::WORKER, {'W', 20, 2, 100, 2, 1, {{ObjectType::BASE, 1}, {ObjectType::KNIGHT, 5}, {ObjectType::SWORDSMAN, 5}, {ObjectType::ARCHER, 5}, {ObjectType::PIKEMAN, 5},
        {ObjectType::RAM, 5}, {ObjectType::CATAPULT, 5}, {ObjectType::WORKER, 5}}}}
    };
}

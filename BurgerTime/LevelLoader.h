#pragma once
#include <string>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "LevelMap.h"

struct LevelTransform
{
    float scaleX, scaleY, offsetX, offsetY;
    float WorldX(float spriteX) const { return offsetX + spriteX * scaleX; }
    float WorldY(float spriteY) const { return offsetY + spriteY * scaleY; }
};

// Grid coordinate helpers (sprite pixel space)
inline constexpr int GridPlatformY(int row) { return 1 + row * 16; }
inline constexpr int GridLadderX(int col)   { return 8 + col * 24; }

struct BurgerPieceDef
{
    int type;     // 0=top_bun, 1=patty, 2=lettuce, 3=bot_bun, 4=tomato, 5=cheese
    int row;      // platform row 0-11
    int startCol; // ladder column (0-8) the piece is centered on
};

struct CupDef
{
    int   row;    // platform row the cup sits just below
    int   col;    // ladder column the cup is centered on
    float worldY; // world-space Y where landed burgers stack (computed at load time)
};

struct EnemySpawnDef
{
    int   type;   // 0=Hotdog, 1=Egg, 2=Pickle  (int to avoid circular include with EnemyComponent.h)
    float spawnX; // sprite-space X  (<0 = off left, >208 = off right)
    float spawnY; // sprite-space Y  (platform row pixel height)
    float delay;  // seconds before the enemy starts moving
};

struct LevelData
{
    std::unique_ptr<dae::LevelMap> map;
    glm::vec2 playerStart{};
    glm::vec2 bonusPos{ -1.f, -1.f }; // world-space; (-1,-1) means no B marker placed yet
    std::vector<BurgerPieceDef>  burgers;
    std::vector<CupDef>          cups;
    std::vector<glm::vec2>       spawnPoints; // sprite-space, sorted BR,BL,TR,TL
    std::vector<EnemySpawnDef>   enemies;
};

class LevelLoader
{
public:
    static LevelData Load(const std::string& filePath, const LevelTransform& transform);
};

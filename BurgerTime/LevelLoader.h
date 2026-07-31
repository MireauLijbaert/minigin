#pragma once
#include <string>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "LevelMap.h"

struct BurgerPieceDef
{
    int type;     // 0=top_bun, 1=patty, 2=lettuce, 3=bot_bun, 4=tomato, 5=cheese
    int row;      // platform row 0-11
    int startCol; // ladder column (0-8) the piece is centered on
};

struct CupDef
{
    int row; // platform row the cup sits just below
    int col; // ladder column the cup is centered on
};

struct LevelData
{
    std::unique_ptr<dae::LevelMap> map;
    glm::ivec2 spriteSize{ 208, 187 };
    glm::vec2 playerStartSprite{ 104.f, 1.f }; // sprite pixel coords (feet center)
    std::vector<BurgerPieceDef> burgers;
    std::vector<CupDef> cups;
};

class LevelLoader
{
public:
    static LevelData Load(const std::string& filePath);
};

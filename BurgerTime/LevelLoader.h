#pragma once
#include <string>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "LevelMap.h"

struct BurgerPieceDef
{
    int type;     // 0=top_bun, 1=patty, 2=lettuce, 3=bot_bun
    int row;      // platform row 0-11
    int startCol; // first of 4 grid columns, 0-5
};

struct LevelData
{
    std::unique_ptr<dae::LevelMap> map;
    glm::ivec2 spriteSize{ 208, 187 };
    glm::vec2 playerStartSprite{ 104.f, 1.f }; // sprite pixel coords (feet center)
    std::vector<BurgerPieceDef> burgers;
};

class LevelLoader
{
public:
    static LevelData Load(const std::string& filePath);
};

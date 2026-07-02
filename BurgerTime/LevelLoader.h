#pragma once
#include <string>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "LevelMap.h"

namespace dae { class Scene; class GameObject; }

enum class IngredientType { TopBun, Patty, Lettuce, BottomBun };

struct IngredientSpawn
{
    glm::ivec2 cell;
    IngredientType type;
};

struct LevelData
{
    std::unique_ptr<dae::LevelMap> map;
    glm::ivec2 gridSize{ 13, 15 };
    glm::ivec2 playerStartCell{ 6, 7 };
    std::vector<glm::ivec2> enemySpawnCells;
    std::vector<IngredientSpawn> ingredients;
    std::vector<glm::ivec2> plateCells;
};

class LevelLoader
{
public:
    static LevelData Load(const std::string& filePath);
};

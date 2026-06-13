#pragma once
#include <string>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "GridRegistry.h"

namespace dae { class Scene; class GameObject; }

struct LevelData
{
    std::unique_ptr<dae::GridRegistry> registry;
    glm::ivec2 playerStartCell{ 6, 6 };
    glm::ivec2 gridSize{ 13, 15 };
    std::vector<glm::ivec2> snoBeeSpawnCells;
    std::vector<glm::ivec2> eggCells;
};

class LevelLoader
{
public:
    static LevelData Load(const std::string& filePath, dae::Scene& scene, int tileSize, dae::GameObject* gridRoot = nullptr);
};

#include "LevelLoader.h"
#include "Scene.h"
#include "GameObject.h"
#include "RenderComponent.h"
#include "GridRegistry.h"
#include <fstream>
#include <string>
#include <algorithm>

LevelData LevelLoader::Load(const std::string& filePath, dae::Scene& scene, int tileSize)
{
    LevelData data;
    data.registry = std::make_unique<dae::GridRegistry>();

    std::ifstream file(filePath);
    if (!file.is_open())
        return data;

    std::string line;
    int row = 0;
    int maxCol = 0;

    while (std::getline(file, line))
    {
        for (int col = 0; col < static_cast<int>(line.size()); ++col)
        {
            const char c = line[col];
            const glm::ivec2 cell{ col, row };

            if (c == 'I' || c == 'E')
            {
                auto obj = std::make_unique<dae::GameObject>();
                auto render = std::make_unique<dae::RenderComponent>(*obj);
                render->SetTexture("IceCube.png");
                obj->AddComponent(std::move(render));
                obj->SetLocalPosition(float(col * tileSize), float(row * tileSize));

                data.registry->Register(cell, obj.get());
                scene.Add(std::move(obj));
            }
            else if (c == 'P')
            {
                data.playerStartCell = cell;
            }

            maxCol = std::max(maxCol, col + 1);
        }
        ++row;
    }

    data.gridSize = { maxCol, row };
    return data;
}

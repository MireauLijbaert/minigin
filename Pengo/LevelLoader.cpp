#include "LevelLoader.h"
#include "Scene.h"
#include "GameObject.h"
#include "RenderComponent.h"
#include "IceBlockComponent.h"
#include "GridRegistry.h"
#include <fstream>
#include <string>
#include <algorithm>

LevelData LevelLoader::Load(const std::string& filePath, dae::Scene& scene, int tileSize, dae::GameObject* gridRoot)
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

            if (c == 'I' || c == 'E' || c == 'D')
            {
                auto obj = std::make_unique<dae::GameObject>();
                auto render = std::make_unique<dae::RenderComponent>(*obj);
                render->SetTexture("IceCube.png");
                obj->AddComponent(std::move(render));
                obj->AddComponent(std::make_unique<dae::IceBlockComponent>(*obj, cell, tileSize));
                obj->SetLocalPosition(float(col * tileSize), float(row * tileSize));
                if (gridRoot)
                    obj->SetParent(gridRoot, false);

                data.registry->Register(cell, obj.get());
                scene.Add(std::move(obj));

                if (c == 'E')
                    data.eggCells.push_back(cell);
            }
            else if (c == 'P')
            {
                data.playerStartCell = cell;
            }
            else if (c == 'S')
            {
                data.snoBeeSpawnCells.push_back(cell);
            }

            maxCol = std::max(maxCol, col + 1);
        }
        ++row;
    }

    data.gridSize = { maxCol, row };
    return data;
}

#include "LevelLoader.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cassert>

LevelData LevelLoader::Load(const std::string& filePath)
{
    std::ifstream file(filePath);
    assert(file.is_open() && "LevelLoader: failed to open file");

    LevelData data;

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line))
        lines.push_back(line);

    int rows = static_cast<int>(lines.size());
    int cols = 0;
    for (const auto& l : lines)
        cols = std::max(cols, static_cast<int>(l.size()));

    cols = std::max(cols, 13);
    rows = std::max(rows, 15);

    data.gridSize = { cols, rows };
    data.map = std::make_unique<dae::LevelMap>(cols, rows);

    for (int y = 0; y < static_cast<int>(lines.size()); ++y)
    {
        const std::string& row = lines[y];
        for (int x = 0; x < static_cast<int>(row.size()); ++x)
        {
            char c = row[x];
            glm::ivec2 cell{ x, y };

            switch (c)
            {
            case '-':
                data.map->SetPlatform(cell, true);
                break;
            case '|':
                data.map->SetLadder(cell, true);
                break;
            case '+':
                data.map->SetPlatform(cell, true);
                data.map->SetLadder(cell, true);
                break;
            case '=':
                data.map->SetPlate(cell, true);
                data.plateCells.push_back(cell);
                break;
            case 'P':
                data.map->SetPlatform(cell, true);
                data.playerStartCell = cell;
                break;
            case 'E':
                data.map->SetPlatform(cell, true);
                data.enemySpawnCells.push_back(cell);
                break;
            case '1':
                data.map->SetPlatform(cell, true);
                data.ingredients.push_back({ cell, IngredientType::TopBun });
                break;
            case '2':
                data.map->SetPlatform(cell, true);
                data.ingredients.push_back({ cell, IngredientType::Patty });
                break;
            case '3':
                data.map->SetPlatform(cell, true);
                data.ingredients.push_back({ cell, IngredientType::Lettuce });
                break;
            case '4':
                data.map->SetPlatform(cell, true);
                data.ingredients.push_back({ cell, IngredientType::BottomBun });
                break;
            default:
                break;
            }
        }
    }

    return data;
}

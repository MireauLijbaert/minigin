#include "LevelLoader.h"
#include <fstream>
#include <cassert>

LevelData LevelLoader::Load(const std::string& filePath, const LevelTransform& transform)
{
    std::ifstream file(filePath);
    assert(file.is_open() && "LevelLoader: failed to open file");

    LevelData data;
    data.map = std::make_unique<dae::LevelMap>();

    // Collect all non-comment lines in order (blank lines are kept they represent empty rows)
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line))
    {
        if (!line.empty() && line[0] == '#') continue;
        lines.push_back(line);
    }

    // Even-indexed lines (0,2,4,...22): platform rows (12 total)
  // Odd-indexed lines  (1,3,5,...21): ladder rows  (11 total, one per gap between platform rows)
    auto cell = [](const std::string& l, int col) -> char {
        int pos = col * 2;
        return pos < static_cast<int>(l.size()) ? l[pos] : ' ';
    };

    // --- Platforms ---
    for (int row = 0; row < 12; ++row)
    {
        int idx = row * 2;
        if (idx >= static_cast<int>(lines.size())) break;
        const std::string& l = lines[idx];

        // Find contiguous groups of active columns and emit a platform per group
        int segStart = -1;
        for (int col = 0; col <= 9; ++col)
        {
            char c = (col < 9) ? cell(l, col) : ' ';
            bool active = (c != ' ' && c != 'C');

            if (active && segStart < 0)
                segStart = col;

            if (!active && segStart >= 0)
            {
                int lastCol = col - 1;
                float x0 = transform.WorldX(static_cast<float>(GridLadderX(segStart)));
                float x1 = transform.WorldX(static_cast<float>(GridLadderX(lastCol)));
                float y  = transform.WorldY(static_cast<float>(GridPlatformY(row)));
                data.map->AddPlatform(row, y, x0, x1);
                segStart = -1;
            }

            if (c == 'P')
                data.playerStart = {
                    transform.WorldX(static_cast<float>(GridLadderX(col))),
                    transform.WorldY(static_cast<float>(GridPlatformY(row)))
                };

            if (c == 't' || c == 'm' || c == 'l' || c == 'b' || c == 'o' || c == 'c')
            {
                BurgerPieceDef def{};
                if      (c == 't') def.type = 0;
                else if (c == 'm') def.type = 1;
                else if (c == 'l') def.type = 2;
                else if (c == 'b') def.type = 3;
                else if (c == 'o') def.type = 4;
                else               def.type = 5;
                def.row = row;
                def.startCol = col;
                data.burgers.push_back(def);
            }

            if (c == 'C')
                data.cups.push_back({ row, col });
        }
    }

    // --- Ladders ---
    for (int col = 0; col < 9; ++col)
    {
        int segY0Sprite = -1;
        int lastYBotSprite = -1;

        for (int gap = 0; gap < 11; ++gap)
        {
            int idx = gap * 2 + 1;
            bool hasLadr = idx < static_cast<int>(lines.size())
                           && cell(lines[idx], col) == '|';

            int yTop = GridPlatformY(gap);
            int yBot = GridPlatformY(gap + 1);

            if (hasLadr && segY0Sprite < 0)
                segY0Sprite = yTop;
            else if (!hasLadr && segY0Sprite >= 0)
            {
                float x  = transform.WorldX(static_cast<float>(GridLadderX(col)));
                float y0 = transform.WorldY(static_cast<float>(segY0Sprite));
                float y1 = transform.WorldY(static_cast<float>(lastYBotSprite));
                data.map->AddLadder(col, x, y0, y1);
                segY0Sprite = -1;
            }
            lastYBotSprite = yBot;
        }
        if (segY0Sprite >= 0)
        {
            float x  = transform.WorldX(static_cast<float>(GridLadderX(col)));
            float y0 = transform.WorldY(static_cast<float>(segY0Sprite));
            float y1 = transform.WorldY(static_cast<float>(lastYBotSprite));
            data.map->AddLadder(col, x, y0, y1);
        }
    }

    return data;
}

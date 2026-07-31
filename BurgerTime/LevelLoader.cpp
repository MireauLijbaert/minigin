#include "LevelLoader.h"
#include <fstream>
#include <cassert>

// Fixed BurgerTime grid: 12 platform rows x 9 ladder columns
// Row y and column x in sprite pixels, derived from grid index
static constexpr int platformY(int row) { return 1 + row * 16; }
static constexpr int ladderX(int col) { return 8 + col * 24; }
static constexpr int SPRITE_W = 207; // max x (208 wide, 0-indexed)

LevelData LevelLoader::Load(const std::string& filePath)
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
            bool active = (c != ' ');

            if (active && segStart < 0)
                segStart = col;

            if (!active && segStart >= 0)
            {
                int lastCol = col - 1;
                int x0 = (segStart == 0) ? 0 : ladderX(segStart);
                int x1 = (lastCol == 8)  ? SPRITE_W : ladderX(lastCol);
                data.map->AddPlatform(platformY(row), x0, x1);
                segStart = -1;
            }

            if (c == 'P')
                data.playerStartSprite = { static_cast<float>(ladderX(col)),
                                           static_cast<float>(platformY(row)) };

            // t=top_bun  m=patty  g=lettuce  b=bot_bun
            if (c == 't' || c == 'm' || c == 'g' || c == 'b')
            {
                BurgerPieceDef def{};
                if      (c == 't') def.type = 0;
                else if (c == 'm') def.type = 1;
                else if (c == 'g') def.type = 2;
                else               def.type = 3;
                def.row = row;
                def.startCol = col;
                data.burgers.push_back(def);
            }
        }
    }

    // --- Ladders ---
    // For each column, scan the 11 gap rows and merge consecutive '|' entries
    for (int col = 0; col < 9; ++col)
    {
        int segY0 = -1;
        int lastYBot = -1;

        for (int gap = 0; gap < 11; ++gap)
        {
            int idx = gap * 2 + 1;
            bool hasLadr = idx < static_cast<int>(lines.size())
                           && cell(lines[idx], col) == '|';

            int yTop = platformY(gap);
            int yBot = platformY(gap + 1);

            if (hasLadr && segY0 < 0)
                segY0 = yTop;
            else if (!hasLadr && segY0 >= 0)
            {
                data.map->AddLadder(ladderX(col), segY0, lastYBot);
                segY0 = -1;
            }
            lastYBot = yBot;
        }
        if (segY0 >= 0)
            data.map->AddLadder(ladderX(col), segY0, lastYBot);
    }

    return data;
}

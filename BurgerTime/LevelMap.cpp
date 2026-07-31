#include "LevelMap.h"
#include <cmath>

namespace dae
{
    void LevelMap::AddPlatform(int y, int x0, int x1) { m_platforms.push_back({ y, x0, x1 }); }
    void LevelMap::AddLadder(int x, int y0, int y1)   { m_ladders.push_back({ x, y0, y1 }); }

    const PlatformRow* LevelMap::FindPlatform(float spriteX, float spriteY, float threshold) const
    {
        const PlatformRow* best = nullptr;
        float bestDist = threshold + 1.f;
        for (const auto& p : m_platforms)
        {
            float dist = std::abs(spriteY - static_cast<float>(p.y));
            if (dist < bestDist
                && spriteX >= static_cast<float>(p.x0) - threshold
                && spriteX <= static_cast<float>(p.x1) + threshold)
            {
                bestDist = dist;
                best = &p;
            }
        }
        return best;
    }

    const LadderCol* LevelMap::FindLadder(float spriteX, float spriteY, float threshold) const
    {
        const LadderCol* best = nullptr;
        float bestDist = threshold + 1.f;
        for (const auto& l : m_ladders)
        {
            float dist = std::abs(spriteX - static_cast<float>(l.x));
            if (dist < bestDist
                && spriteY >= static_cast<float>(l.y0)
                && spriteY <= static_cast<float>(l.y1))
            {
                bestDist = dist;
                best = &l;
            }
        }
        return best;
    }
}

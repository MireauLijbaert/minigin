#include "LevelMap.h"
#include <cmath>

namespace dae
{
    void LevelMap::AddPlatform(int row, float y, float x0, float x1) { m_platforms.push_back({ row, y, x0, x1 }); }
    void LevelMap::AddLadder(int col, float x, float y0, float y1)   { m_ladders.push_back({ col, x, y0, y1 }); }

    const PlatformRow* LevelMap::FindPlatform(float x, float y, float threshold) const
    {
        const PlatformRow* best = nullptr;
        float bestDist = threshold + 0.001f;
        for (const auto& p : m_platforms)
        {
            float dist = std::abs(y - p.y);
            if (dist < bestDist && x >= p.x0 - 0.001f && x <= p.x1 + 0.001f)
            {
                bestDist = dist;
                best = &p;
            }
        }
        return best;
    }

    const LadderCol* LevelMap::FindLadder(float x, float y, float threshold) const
    {
        const LadderCol* best = nullptr;
        float bestDist = threshold + 0.001f;
        for (const auto& l : m_ladders)
        {
            float dist = std::abs(x - l.x);
            if (dist < bestDist && y >= l.y0 && y <= l.y1)
            {
                bestDist = dist;
                best = &l;
            }
        }
        return best;
    }

    const PlatformRow* LevelMap::FindNextPlatformBelow(float x, float y) const
    {
        const PlatformRow* best = nullptr;
        for (const auto& p : m_platforms)
        {
            if (p.y > y && x >= p.x0 - 0.001f && x <= p.x1 + 0.001f)
            {
                if (!best || p.y < best->y)
                    best = &p;
            }
        }
        return best;
    }
}

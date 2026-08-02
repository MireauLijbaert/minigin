#pragma once
#include <vector>

namespace dae
{
    struct PlatformRow { int row; float y, x0, x1; };
    struct LadderCol   { int col; float x, y0, y1; };

    class LevelMap
    {
    public:
        void AddPlatform(int row, float y, float x0, float x1);
        void AddLadder(int col, float x, float y0, float y1);

        const std::vector<PlatformRow>& GetPlatforms() const { return m_platforms; }
        const std::vector<LadderCol>&   GetLadders()   const { return m_ladders; }

        const PlatformRow* FindPlatform(float x, float y, float threshold) const;
        const LadderCol*   FindLadder(float x, float y, float threshold) const;
        const PlatformRow* FindNextPlatformBelow(float x, float y) const;

    private:
        std::vector<PlatformRow> m_platforms;
        std::vector<LadderCol>   m_ladders;
    };
}

#pragma once
#include <vector>

namespace dae
{
    struct PlatformRow { int y, x0, x1; };
    struct LadderCol   { int x, y0, y1; };

    class LevelMap
    {
    public:
        void AddPlatform(int y, int x0, int x1);
        void AddLadder(int x, int y0, int y1);

        const std::vector<PlatformRow>& GetPlatforms() const { return m_platforms; }
        const std::vector<LadderCol>&   GetLadders()   const { return m_ladders; }

        // Returns nearest platform within threshold whose x range covers spriteX, or nullptr
        const PlatformRow* FindPlatform(float spriteX, float spriteY, float threshold) const;
        // Returns nearest ladder within threshold whose y range covers spriteY, or nullptr
        const LadderCol*   FindLadder(float spriteX, float spriteY, float threshold) const;

    private:
        std::vector<PlatformRow> m_platforms;
        std::vector<LadderCol>   m_ladders;
    };
}

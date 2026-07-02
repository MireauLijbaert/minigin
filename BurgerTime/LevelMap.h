#pragma once
#include <glm/glm.hpp>
#include <vector>

namespace dae
{
    struct CellFlags
    {
        bool platform = false;
        bool ladder = false;
        bool plate = false;
    };

    class LevelMap
    {
    public:
        LevelMap(int cols, int rows);

        bool HasPlatform(glm::ivec2 cell) const;
        bool HasLadder(glm::ivec2 cell) const;
        bool HasPlate(glm::ivec2 cell) const;

        void SetPlatform(glm::ivec2 cell, bool v);
        void SetLadder(glm::ivec2 cell, bool v);
        void SetPlate(glm::ivec2 cell, bool v);

        glm::ivec2 GetSize() const { return m_Size; }
        bool InBounds(glm::ivec2 cell) const;

    private:
        CellFlags& At(glm::ivec2 cell);
        const CellFlags& At(glm::ivec2 cell) const;

        std::vector<CellFlags> m_Cells;
        glm::ivec2 m_Size;
    };
}

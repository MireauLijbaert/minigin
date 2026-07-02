#include "LevelMap.h"
#include <stdexcept>

namespace dae
{
    LevelMap::LevelMap(int cols, int rows)
        : m_Cells(cols * rows)
        , m_Size{ cols, rows }
    {
    }

    bool LevelMap::InBounds(glm::ivec2 cell) const
    {
        return cell.x >= 0 && cell.x < m_Size.x
            && cell.y >= 0 && cell.y < m_Size.y;
    }

    CellFlags& LevelMap::At(glm::ivec2 cell)
    {
        return m_Cells[cell.y * m_Size.x + cell.x];
    }

    const CellFlags& LevelMap::At(glm::ivec2 cell) const
    {
        return m_Cells[cell.y * m_Size.x + cell.x];
    }

    bool LevelMap::HasPlatform(glm::ivec2 cell) const
    {
        return InBounds(cell) && At(cell).platform;
    }

    bool LevelMap::HasLadder(glm::ivec2 cell) const
    {
        return InBounds(cell) && At(cell).ladder;
    }

    bool LevelMap::HasPlate(glm::ivec2 cell) const
    {
        return InBounds(cell) && At(cell).plate;
    }

    void LevelMap::SetPlatform(glm::ivec2 cell, bool v)
    {
        if (InBounds(cell)) At(cell).platform = v;
    }

    void LevelMap::SetLadder(glm::ivec2 cell, bool v)
    {
        if (InBounds(cell)) At(cell).ladder = v;
    }

    void LevelMap::SetPlate(glm::ivec2 cell, bool v)
    {
        if (InBounds(cell)) At(cell).plate = v;
    }
}

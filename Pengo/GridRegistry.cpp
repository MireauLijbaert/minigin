#include "GridRegistry.h"

namespace dae
{
    void GridRegistry::Register(glm::ivec2 cell, GameObject* obj)
    {
        m_Cells[cell] = obj;
    }

    void GridRegistry::Unregister(glm::ivec2 cell)
    {
        m_Cells.erase(cell);
    }

    void GridRegistry::Move(glm::ivec2 from, glm::ivec2 to)
    {
        auto it = m_Cells.find(from);
        if (it != m_Cells.end())
        {
            m_Cells[to] = it->second;
            m_Cells.erase(it);
        }
    }

    GameObject* GridRegistry::GetAt(glm::ivec2 cell) const
    {
        auto it = m_Cells.find(cell);
        return it != m_Cells.end() ? it->second : nullptr;
    }

    bool GridRegistry::IsEmpty(glm::ivec2 cell) const
    {
        return m_Cells.find(cell) == m_Cells.end();
    }
}

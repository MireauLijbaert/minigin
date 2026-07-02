#pragma once
#include <unordered_map>
#include <glm/glm.hpp>

namespace dae
{
    class GameObject;

    class GridRegistry
    {
    public:
        void Register(glm::ivec2 cell, GameObject* obj);
        void Unregister(glm::ivec2 cell);
        void Move(glm::ivec2 from, glm::ivec2 to);

        GameObject* GetAt(glm::ivec2 cell) const;
        bool IsEmpty(glm::ivec2 cell) const;

    private:
        struct IVec2Hash
        {
            size_t operator()(const glm::ivec2& v) const
            {
                return std::hash<int>{}(v.x) ^ (std::hash<int>{}(v.y) << 16);
            }
        };

        std::unordered_map<glm::ivec2, GameObject*, IVec2Hash> m_Cells;
    };
}

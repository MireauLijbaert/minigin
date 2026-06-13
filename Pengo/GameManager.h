#pragma once
#include "BaseComponent.h"
#include "Observer.h"
#include <glm/glm.hpp>
#include <vector>
#include <functional>

namespace dae
{
    class TextComponent;
    class SnoBeeComponent;
    class PengoControllerComponent;
    class GameObject;
    class GridRegistry;

    class GameManager final : public BaseComponent, public Observer
    {
    public:
        GameManager(GameObject& owner, TextComponent* victoryText = nullptr);
        ~GameManager() = default;
        GameManager(const GameManager&)            = delete;
        GameManager(GameManager&&)                 = delete;
        GameManager& operator=(const GameManager&) = delete;
        GameManager& operator=(GameManager&&)      = delete;

        void Update() override;
        void Render() override {}

        void Notify(const Event& event, GameObject* actor) override;

        using SnoBeeSpawnFn = std::function<void(glm::ivec2)>;

        void SetPlayer(PengoControllerComponent* pengo, glm::ivec2 spawnCell);
        void AddSnoBee(SnoBeeComponent* snobee, glm::ivec2 spawnCell);
        void OnSnoBeeKilled(SnoBeeComponent* snobee);

        void SetRegistry(GridRegistry* registry) { m_Registry = registry; }
        void AddEggCell(glm::ivec2 cell) { m_EggCells.push_back(cell); }
        void SetSnoBeeSpawnFn(SnoBeeSpawnFn fn) { m_SpawnFn = std::move(fn); }

        bool IsLevelCleared() const { return m_LevelCleared; }
        bool IsFrenzy()       const { return m_Frenzy; }
        float GetBreakDuration() const;
        void StunNearWall(glm::ivec2 facingDir, glm::ivec2 gridSize);

        // Chase concurrency
        bool CanStartChasing() const  { return m_ActiveChasers < 2; }
        void RegisterChaseStart()     { ++m_ActiveChasers; }
        void RegisterChaseEnd()       { if (m_ActiveChasers > 0) --m_ActiveChasers; }

    private:
        void DoRespawn();
        void DoGameOver(GameObject* player);
        void HatchRandomEgg();

        struct SnoBeeEntry { SnoBeeComponent* component; glm::ivec2 spawnCell; };

        PengoControllerComponent* m_Pengo{ nullptr };
        glm::ivec2 m_PlayerSpawn{};
        std::vector<SnoBeeEntry> m_SnoBees;
        int   m_SnoBeesRemaining{ 0 };
        int   m_ActiveChasers{ 0 };
        bool  m_LevelCleared{ false };
        bool  m_GameOver{ false };
        float m_LevelTimer{ 0.f };
        bool  m_Frenzy{ false };
        TextComponent* m_VictoryText{ nullptr };

        GridRegistry* m_Registry{ nullptr };
        std::vector<glm::ivec2> m_EggCells;
        SnoBeeSpawnFn m_SpawnFn;
    };
}

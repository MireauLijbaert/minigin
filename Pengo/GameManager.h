#pragma once
#include "BaseComponent.h"
#include "Observer.h"
#include <glm/glm.hpp>
#include <vector>

namespace dae
{
    class TextComponent;
    class SnoBeeComponent;
    class GameObject;

    class GameManager final : public BaseComponent, public Observer
    {
    public:
        GameManager(GameObject& owner, TextComponent* victoryText = nullptr);
        ~GameManager() = default;
        GameManager(const GameManager&)            = delete;
        GameManager(GameManager&&)                 = delete;
        GameManager& operator=(const GameManager&) = delete;
        GameManager& operator=(GameManager&&)      = delete;

        void Update() override {}
        void Render() override {}

        // Observer, reacts to player LifeChanged event
        void Notify(const Event& event, GameObject* actor) override;

        void SetPlayer(GameObject* player, glm::ivec2 spawnCell);
        void AddSnoBee(SnoBeeComponent* snobee, glm::ivec2 spawnCell);
        void OnSnoBeeKilled(SnoBeeComponent* snobee);

        bool IsLevelCleared() const { return m_LevelCleared; }

    private:
        void Respawn();
        void GameOver(GameObject* player);

        struct SnoBeeEntry { SnoBeeComponent* component; glm::ivec2 spawnCell; };

        GameObject* m_Player{ nullptr };
        glm::ivec2 m_PlayerSpawn{};
        std::vector<SnoBeeEntry> m_SnoBees;
        int m_SnoBeesRemaining{ 0 };
        bool m_LevelCleared{ false };
        TextComponent* m_VictoryText{ nullptr };
    };
}

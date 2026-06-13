#pragma once
#include "BaseComponent.h"
#include "Observer.h"
#include <glm/glm.hpp>
#include <vector>

namespace dae
{
    class TextComponent;
    class SnoBeeComponent;
    class PengoControllerComponent;
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

        void Notify(const Event& event, GameObject* actor) override;

        void SetPlayer(PengoControllerComponent* pengo, glm::ivec2 spawnCell);
        void AddSnoBee(SnoBeeComponent* snobee, glm::ivec2 spawnCell);
        void OnSnoBeeKilled(SnoBeeComponent* snobee);

        bool IsLevelCleared() const { return m_LevelCleared; }

    private:
        void DoRespawn();
        void DoGameOver(GameObject* player);

        struct SnoBeeEntry { SnoBeeComponent* component; glm::ivec2 spawnCell; };

        PengoControllerComponent* m_Pengo{ nullptr };
        glm::ivec2 m_PlayerSpawn{};
        std::vector<SnoBeeEntry> m_SnoBees;
        int m_SnoBeesRemaining{ 0 };
        bool m_LevelCleared{ false };
        TextComponent* m_VictoryText{ nullptr };
    };
}

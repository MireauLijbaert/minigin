#pragma once
#include "BaseComponent.h"

namespace dae
{
    class TextComponent;

    class GameManager final : public BaseComponent
    {
    public:
        GameManager(GameObject& owner, int snoBeeCount, TextComponent* victoryText = nullptr);
        ~GameManager() = default;
        GameManager(const GameManager&)            = delete;
        GameManager(GameManager&&)                 = delete;
        GameManager& operator=(const GameManager&) = delete;
        GameManager& operator=(GameManager&&)      = delete;

        void Update() override {}
        void Render() override {}

        void OnSnoBeeKilled();

        bool IsLevelCleared() const { return m_LevelCleared; }

    private:
        int            m_SnoBeesRemaining;
        bool           m_LevelCleared{ false };
        TextComponent* m_VictoryText{ nullptr };
    };
}

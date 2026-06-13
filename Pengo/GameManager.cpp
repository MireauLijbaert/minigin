#include "GameManager.h"
#include "TextComponent.h"

namespace dae
{
    GameManager::GameManager(GameObject& owner, int snoBeeCount, TextComponent* victoryText)
        : BaseComponent(owner)
        , m_SnoBeesRemaining{ snoBeeCount }
        , m_VictoryText{ victoryText }
    {
    }

    void GameManager::OnSnoBeeKilled()
    {
        if (m_LevelCleared) return;

        --m_SnoBeesRemaining;

        if (m_SnoBeesRemaining <= 0)
        {
            m_LevelCleared = true;
            if (m_VictoryText)
                m_VictoryText->SetText("LEVEL CLEARED!");
        }
    }
}

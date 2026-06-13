#include "GameManager.h"
#include "TextComponent.h"
#include "SnoBeeComponent.h"
#include "HealthComponent.h"
#include "GridMovementComponent.h"
#include "GameObject.h"
#include "Event.h"

namespace dae
{
    GameManager::GameManager(GameObject& owner, TextComponent* victoryText)
        : BaseComponent(owner)
        , m_VictoryText{ victoryText }
    {
    }

    void GameManager::SetPlayer(GameObject* player, glm::ivec2 spawnCell)
    {
        m_Player      = player;
        m_PlayerSpawn = spawnCell;

        if (auto* health = player->GetComponent<HealthComponent>())
            health->GetSubject().AddObserver(this);
    }

    void GameManager::AddSnoBee(SnoBeeComponent* snobee, glm::ivec2 spawnCell)
    {
        m_SnoBees.push_back({ snobee, spawnCell });
        ++m_SnoBeesRemaining;
    }

    void GameManager::OnSnoBeeKilled(SnoBeeComponent* snobee)
    {
        if (m_LevelCleared) return;

        // Nullify the entry so Respawn skips this one
        for (auto& entry : m_SnoBees)
            if (entry.component == snobee) { entry.component = nullptr; break; }

        if (--m_SnoBeesRemaining <= 0)
        {
            m_LevelCleared = true;
            if (m_VictoryText)
                m_VictoryText->SetText("LEVEL CLEARED!");
        }
    }

    void GameManager::Notify(const Event& event, GameObject* actor)
    {
        if (event.id != "LifeChanged") return;

        const int livesLeft = event.args[0].intValue;

        if (livesLeft > 0)
            Respawn();
        else
            GameOver(actor);
    }

    void GameManager::Respawn()
    {
        // Reset player position
        if (m_Player)
            if (auto* mov = m_Player->GetComponent<GridMovementComponent>())
                mov->Respawn(m_PlayerSpawn);

        // Reset every Sno-bee that is still alive (non-null entry)
        for (auto& entry : m_SnoBees)
            if (entry.component)
                entry.component->Respawn(entry.spawnCell);
    }

    void GameManager::GameOver(GameObject* player)
    {
        if (m_VictoryText)
            m_VictoryText->SetText("GAME OVER");

        if (player)
            player->MarkForRemoval();
    }
}

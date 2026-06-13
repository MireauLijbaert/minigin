#include "GameManager.h"
#include "TextComponent.h"
#include "SnoBeeComponent.h"
#include "PengoControllerComponent.h"
#include "HealthComponent.h"
#include "GameObject.h"
#include "Event.h"

namespace dae
{
    GameManager::GameManager(GameObject& owner, TextComponent* victoryText)
        : BaseComponent(owner)
        , m_VictoryText{ victoryText }
    {
    }

    void GameManager::SetPlayer(PengoControllerComponent* pengo, glm::ivec2 spawnCell)
    {
        m_Pengo       = pengo;
        m_PlayerSpawn = spawnCell;

        if (auto* health = pengo->GetOwner()->GetComponent<HealthComponent>())
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
            DoRespawn();
        else
            DoGameOver(actor);
    }

    void GameManager::DoRespawn()
    {
        if (m_Pengo)
            m_Pengo->Respawn(m_PlayerSpawn);

        for (auto& entry : m_SnoBees)
            if (entry.component)
                entry.component->Respawn(entry.spawnCell);
    }

    void GameManager::DoGameOver(GameObject* player)
    {
        if (m_VictoryText)
            m_VictoryText->SetText("GAME OVER");

        if (player)
            player->MarkForRemoval();
    }
}

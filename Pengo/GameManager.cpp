#include "GameManager.h"
#include "TextComponent.h"
#include "SnoBeeComponent.h"
#include "PengoControllerComponent.h"
#include "HealthComponent.h"
#include "GridMovementComponent.h"
#include "GameObject.h"
#include "Event.h"
#include "TimeSingleton.h"

namespace dae
{
    GameManager::GameManager(GameObject& owner, TextComponent* victoryText)
        : BaseComponent(owner)
        , m_VictoryText{ victoryText }
    {
    }

    void GameManager::Update()
    {
        if (m_LevelCleared || m_GameOver) return;

        m_LevelTimer += Time::GetInstance().GetDeltaTime();
        if (!m_Frenzy && m_LevelTimer >= 120.f)
            m_Frenzy = true;
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

    float GameManager::GetBreakDuration() const
    {
        // Normal: 3s. Frenzy (2 minute mark): 0.3s
        return m_Frenzy ? 0.3f : 3.0f;
    }

    void GameManager::StunNearWall(glm::ivec2 facingDir, glm::ivec2 gridSize)
    {
        for (auto& entry : m_SnoBees)
        {
            if (!entry.component) continue;
            auto* mov = entry.component->GetMovement();
            if (!mov) continue;
            const glm::ivec2 pos = mov->GetGridPos();

            bool onWall = false;
            if      (facingDir.x < 0) onWall = (pos.x == 0);
            else if (facingDir.x > 0) onWall = (pos.x == gridSize.x - 1);
            else if (facingDir.y < 0) onWall = (pos.y == 0);
            else if (facingDir.y > 0) onWall = (pos.y == gridSize.y - 1);

            if (onWall)
                entry.component->Stun();
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
        m_ActiveChasers = 0;

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

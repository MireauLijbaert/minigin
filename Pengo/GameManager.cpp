#include "GameManager.h"
#include "TextComponent.h"
#include "SnoBeeComponent.h"
#include "PengoControllerComponent.h"
#include "HealthComponent.h"
#include "GridMovementComponent.h"
#include "GridRegistry.h"
#include "GameObject.h"
#include "Event.h"
#include "TimeSingleton.h"
#include "ServiceLocator.h"
#include "PengoSounds.h"
#include <algorithm>
#include <cstdlib>

namespace dae
{
    GameManager::GameManager(GameObject& owner, TextComponent* victoryText)
        : BaseComponent(owner)
        , m_VictoryText{ victoryText }
    {
    }

    void GameManager::Update()
    {
        if (m_GameOver) return;

        if (m_LevelCleared)
        {
            m_LevelClearDelay += Time::GetInstance().GetDeltaTime();
            if (m_LevelClearDelay >= 3.0f && m_OnLevelComplete)
            {
                auto fn = std::move(m_OnLevelComplete);
                fn(); // calls RequestLoad, deferred to next frame, safe
            }
            return;
        }

        m_LevelTimer += Time::GetInstance().GetDeltaTime();
        if (!m_Frenzy && m_LevelTimer >= 60.f) {
            m_Frenzy = true;
            ServiceLocator::GetSoundSystem().PlayMusic(PengoSounds::BGM_FAST, -1);
            ServiceLocator::GetSoundSystem().SetMusicVolume(PengoSounds::MUSIC_VOLUME);
        }
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

        --m_SnoBeesRemaining;

        // Hatch a replacement from a random egg before checking win condition
        HatchRandomEgg();

        if (m_SnoBeesRemaining == 1)
            ServiceLocator::GetSoundSystem().Play(PengoSounds::ONE_BEE_LEFT, PengoSounds::FULL_VOLUME);

        if (m_SnoBeesRemaining <= 0)
        {
            m_LevelCleared = true;
            ServiceLocator::GetSoundSystem().StopMusic();
            ServiceLocator::GetSoundSystem().Play(PengoSounds::ACT_CLEAR, PengoSounds::FULL_VOLUME);
            if (m_VictoryText)
                m_VictoryText->SetText("LEVEL CLEARED!");
        }
    }

    float GameManager::GetBreakDuration() const
    {
        return 0.5f;
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

        ServiceLocator::GetSoundSystem().Play(PengoSounds::MISS, PengoSounds::FULL_VOLUME);

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

    void GameManager::HatchRandomEgg()
    {
        if (!m_SpawnFn || !m_Registry) return;

        // Prune cells whose egg was already destroyed by a sno-bee
        m_EggCells.erase(
            std::remove_if(m_EggCells.begin(), m_EggCells.end(),
                [this](glm::ivec2 pos) { return m_Registry->IsEmpty(pos); }),
            m_EggCells.end()
        );

        if (m_EggCells.empty()) return;

        // Pick and remove a random egg cell
        int idx = rand() % static_cast<int>(m_EggCells.size());
        glm::ivec2 pos = m_EggCells[idx];
        m_EggCells.erase(m_EggCells.begin() + idx);

        // Destroy the egg block
        if (auto* egg = m_Registry->GetAt(pos))
        {
            m_Registry->Unregister(pos);
            egg->MarkForRemoval();
        }

        ServiceLocator::GetSoundSystem().Play(PengoSounds::EGG_HATCH, PengoSounds::FULL_VOLUME);

        // Spawn a new sno-bee at the hatched position
        m_SpawnFn(pos);
    }
}

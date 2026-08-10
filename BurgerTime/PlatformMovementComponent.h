#pragma once
#include "BaseComponent.h"
#include "Subject.h"
#include "LevelMap.h"
#include "LevelLoader.h"
#include "HealthComponent.h"
#include <glm/glm.hpp>
#include <vector>

class EnemyComponent;

class PlatformMovementComponent : public dae::BaseComponent
{
public:
    dae::Subject& GetSubject() { return m_subject; }
    PlatformMovementComponent(dae::GameObject& owner,
                              const dae::LevelMap* levelMap,
                              glm::vec2 worldPos,
                              float charWorldW, float charWorldH,
                              const LevelTransform& transform);

    void Update() override;
    void Render() override {}

    float GetPosX() const { return m_posX; }
    float GetPosY() const { return m_posY; }
    glm::vec2 GetFacingDir() const { return m_facingDir; }

    bool IsAlive()  const { return m_state == PlayerState::Alive; }
    bool IsDying()  const { return m_state == PlayerState::Dying; }
    bool IsMoving() const { return m_isMoving; }
    void Kill();
    // Halt movement input for a fixed duration (used by pepper throw)
    void FreezeFor(float duration) { m_freezeTimer = duration; m_isMoving = false; }
    void SetHealthComponent(dae::HealthComponent* health) { m_health = health; }
    void SetEnemies(std::vector<EnemyComponent*>* enemies) { m_enemies = enemies; }

private:
    enum class PlayerState { Alive, Dying, Dead };
    PlayerState m_state{ PlayerState::Alive };
    float m_respawnTimer{ 0.f };
    float m_deathTimer{ 0.f };
    float m_freezeTimer{ 0.f };
    bool  m_isMoving{ false };
    glm::vec2 m_startPos;
    glm::vec2 m_facingDir{ 1.f, 0.f };
    dae::HealthComponent* m_health{ nullptr };
    std::vector<EnemyComponent*>* m_enemies{ nullptr };

    const dae::LevelMap* m_levelMap;

    float m_posX, m_posY;
    float m_charHalfW;
    float m_charRenderH;
    float m_stepX, m_stepY;
    float m_platThresh;
    float m_ladrThresh;

    float m_stepTimer{ 0.f };

    static constexpr float STEP_INTERVAL       = 1.f / 60.f;
    static constexpr float DEATH_ANIM_DURATION = 1.2f;  // 6 frames @ ~5fps
    static constexpr float RESPAWN_DELAY       = 2.f;

    dae::Subject m_subject;
    void SyncPosition();
};

#pragma once
#include "BaseComponent.h"
#include "Subject.h"
#include "LevelMap.h"
#include "LevelLoader.h"
#include "HealthComponent.h"
#include <glm/glm.hpp>
#include <vector>
#include <SDL3/SDL.h>

class EnemyComponent;

// Configurable key bindings for a player (defaults to WASD)
struct PlayerKeys
{
    SDL_Scancode up    = SDL_SCANCODE_W;
    SDL_Scancode down  = SDL_SCANCODE_S;
    SDL_Scancode left  = SDL_SCANCODE_A;
    SDL_Scancode right = SDL_SCANCODE_D;
};

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
    void SetKeys(const PlayerKeys& keys) { m_keys = keys; }
    // Enable gamepad movement for this player (XInput index 0-3)
    void SetGamepad(bool use, uint32_t index = 0) { m_useGamepad = use; m_gamepadIndex = index; }

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
    PlayerKeys m_keys{};
    bool     m_useGamepad{ false };
    uint32_t m_gamepadIndex{ 0 };

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

#pragma once
#include "BaseComponent.h"
#include "LevelMap.h"
#include "LevelLoader.h"
#include <glm/glm.hpp>

class PlatformMovementComponent;

enum class EnemyType { Hotdog, Egg, Pickle };

class EnemyComponent : public dae::BaseComponent
{
public:
    EnemyComponent(dae::GameObject& owner,
                   const dae::LevelMap* levelMap,
                   glm::vec2 worldPos,
                   float charWorldW, float charWorldH,
                   const LevelTransform& transform,
                   PlatformMovementComponent* player,
                   EnemyType type = EnemyType::Hotdog);

    int GetSquishScore() const
    {
        switch (m_type)
        {
        case EnemyType::Egg:    return 200;
        case EnemyType::Pickle: return 300;
        default:                return 100;
        }
    }

    void Update() override;
    void Render() override {}

    float GetPosX() const { return m_posX; }
    float GetPosY() const { return m_posY; }

    // Returns true when the enemy is active (can interact with burgers/player)
    bool IsAlive() const { return m_state == State::Entering || m_state == State::Walking || m_state == State::Stunned || m_state == State::Waiting; }

    // Called by PepperComponent
    void Stun();

    // Called by BurgerPieceComponent when a burger starts falling on this enemy's row
    void CatchByBurger();

    // Called by BurgerPieceComponent each frame while carrying this enemy
    void SetFallingY(float y);

    // Called by BurgerPieceComponent on landing (squish or end of fall)
    void Kill();

    // Snap back to spawn immediately, called when player dies
    // delay: how long to freeze before walking (should match player respawn delay)
    void Reset(float delay = 0.f);

private:
    enum class State { Entering, Walking, Stunned, FallingWithBurger, Dead, Waiting };

    const dae::LevelMap* m_levelMap;
    PlatformMovementComponent* m_player;
    EnemyType m_type;

    float m_posX, m_posY;
    float m_charHalfW;
    float m_charRenderH;

    glm::vec2 m_MovementDirection{ 0.f, 0.f };
    glm::vec2 m_spawnPos;

    State m_state{ State::Walking };
    float m_stateTimer{ 0.f };

    float m_intersectionCooldown{ 0.f };
    float m_speed;
    float m_platSnap, m_ladrSnap, m_interThresh;
    float m_hitRadiusSq; // collision distance squared for player touch

    static constexpr float SPEED_SPRITE          = 10.f;
    static constexpr float INTERSECTION_COOLDOWN  = 0.15f;
    static constexpr float STUN_DURATION          = 3.f;
    static constexpr float RESPAWN_DELAY          = 4.f;

    void UpdateWalking(float dt);
    void SyncPosition();
};

#pragma once
#include "BaseComponent.h"
#include "Subject.h"
#include "LevelMap.h"
#include "LevelLoader.h"
#include <glm/glm.hpp>

class PlatformMovementComponent;

enum class EnemyType { Hotdog, Egg, Pickle };

class EnemyComponent : public dae::BaseComponent
{
public:
    dae::Subject& GetSubject() { return m_subject; }
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

    // Animation helpers
    glm::vec2 GetMovementDir() const { return m_MovementDirection; }
    bool IsFacingRight() const { return m_MovementDirection.x >= 0.f; }
    bool IsFallingWithBurger() const { return m_state == State::FallingWithBurger; }
    bool IsStunnedAnim()  const { return m_state == State::Stunned; }
    bool IsSquishedAnim() const { return m_state == State::Squished; }
    // Dead and RecoveringFromBurger are both off-screen,  animator skips both
    bool IsDeadAnim()     const { return m_state == State::Dead
                                      || m_state == State::RecoveringFromBurger; }

    // Called by PepperComponent
    void Stun();

    // Called by BurgerPieceComponent when a burger starts falling on this enemy's row
    void CatchByBurger();

    // Called by BurgerPieceComponent each frame while carrying this enemy
    void SetFallingY(float y);

    // Called by BurgerPieceComponent on landing, plays squish anim then dies
    void Squish();

    // Instant death (e.g. hit by burger mid-air on a ladder)
    void Kill();

    // Called for enemies CARRIED by a burger when it lands.
    // They don't die, they recover briefly then resume normal movement.
    void RecoverFromBurger(float landingY);

    // Snap back to spawn immediately, called when player dies
    // delay: how long to freeze before walking (should match player respawn delay)
    void Reset(float delay = 0.f);

    // Level-clear freeze: stops all movement; animator will pause the sprite
    void Freeze() { m_levelClearFrozen = true; }
    bool IsLevelClearFrozen() const { return m_levelClearFrozen; }

private:
    enum class State { Entering, Walking, Stunned, FallingWithBurger, Dead, Waiting,
                       Squished,             // plays squish animation then dies
                       RecoveringFromBurger, // brief freeze at landing spot before resuming
                       ClimbingFromCup };    // climbing up a ladder after landing in a cup

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

    static constexpr float SPEED_SPRITE           = 10.f;
    static constexpr float INTERSECTION_COOLDOWN  = 0.15f;
    static constexpr float STUN_DURATION          = 3.f;
    static constexpr float RESPAWN_DELAY          = 4.f;
    static constexpr float BURGER_RECOVERY_DELAY  = 1.5f;
    static constexpr float SQUISH_ANIM_DURATION   = 0.8f; // 4 frames @ ~5fps

    bool m_levelClearFrozen{ false };
    dae::Subject m_subject;
    void UpdateWalking(float dt);
    void SyncPosition();
};

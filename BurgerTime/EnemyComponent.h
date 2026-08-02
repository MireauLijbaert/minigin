#pragma once
#include "BaseComponent.h"
#include "LevelMap.h"
#include "LevelLoader.h"
#include <glm/glm.hpp>

class PlatformMovementComponent;

class EnemyComponent : public dae::BaseComponent
{
public:
    EnemyComponent(dae::GameObject& owner,
                   const dae::LevelMap* levelMap,
                   glm::vec2 worldPos,
                   float charWorldW, float charWorldH,
                   const LevelTransform& transform,
                   PlatformMovementComponent* player);

    void Update() override;
    void Render() override {}

    float GetPosX() const { return m_posX; }
    float GetPosY() const { return m_posY; }

private:
    const dae::LevelMap* m_levelMap;
    PlatformMovementComponent* m_player;

    float m_posX, m_posY;
    float m_charHalfW;
    float m_charRenderH;

    glm::vec2 m_MovementDirection{ 0.f, 0.f }; // x: -1 left, 1 right; y: -1 down, 1 up

    float m_intersectionCooldown{ 0.f };
    float m_speed;
    float m_platSnap, m_ladrSnap, m_interThresh;

    static constexpr float SPEED_SPRITE          = 10.f;  // sprite px/sec (test speed)
    static constexpr float INTERSECTION_COOLDOWN = 0.15f;

    void SyncPosition();
};

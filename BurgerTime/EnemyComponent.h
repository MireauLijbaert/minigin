#pragma once
#include "BaseComponent.h"
#include "LevelMap.h"
#include <glm/glm.hpp>

class PlatformMovementComponent;

class EnemyComponent : public dae::BaseComponent
{
public:
    EnemyComponent(dae::GameObject& owner,
                   const dae::LevelMap* levelMap,
                   glm::vec2 startSpritePos,
                   float scaleX, float scaleY,
                   float offsetX, float offsetY,
                   float spriteW, float spriteH,
                   PlatformMovementComponent* player);

    void Update() override;
    void Render() override {}

    float GetSpritePosX() const { return m_spritePosX; }
    float GetSpritePosY() const { return m_spritePosY; }

private:
    const dae::LevelMap* m_levelMap;
    PlatformMovementComponent* m_player;

    float m_spritePosX, m_spritePosY;
    float m_scaleX, m_scaleY;
    float m_offsetX, m_offsetY;
    float m_spriteW, m_spriteH;

	glm::vec2 m_MovementDirection{ 0.f, 0.f }; // x: -1 left, 1 right, 0 none; y: -1 down, 1 up, 0 none (can't have both x and y non-zero at the same time)

    float m_intersectionCooldown{ 0.f }; // prevents re-triggering the same intersection

    static constexpr float SPEED                = 10.f;
    static constexpr float INTERSECTION_COOLDOWN = 0.15f; // seconds to ignore intersections after taking one
    static constexpr float PLAT_SNAP            = 2.f;
    static constexpr float LADR_SNAP            = 4.f;
    static constexpr float INTER_THRESH         = 0.5f;

    void SyncWorldPosition();
};

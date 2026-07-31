#pragma once
#include "BaseComponent.h"
#include "LevelMap.h"
#include <glm/glm.hpp>

class PlatformMovementComponent : public dae::BaseComponent
{
public:
    // spritePos: starting feet-center in sprite pixel coords
    // scaleX/Y: sprite pixel to world pixel conversion
    // offsetX/Y: world position of sprite origin (top-left of level)
    // spriteW/H: native size of the character sprite in pixels
    PlatformMovementComponent(dae::GameObject& owner,
                              const dae::LevelMap* levelMap,
                              glm::vec2 spritePos,
                              float scaleX, float scaleY,
                              float offsetX, float offsetY,
                              float spriteW, float spriteH);

    void Update() override;
    void Render() override {}

    float GetSpritePosX() const { return m_spritePosX; }
    float GetSpritePosY() const { return m_spritePosY; }

private:
    const dae::LevelMap* m_levelMap;

    float m_spritePosX, m_spritePosY; // feet-center in sprite coords
    float m_scaleX, m_scaleY;
    float m_offsetX, m_offsetY;
    float m_spriteW, m_spriteH;

    float m_stepTimer{ 0.f }; // counts up to STEP_INTERVAL before taking a step

    static constexpr float STEP           = 1.f;          // 1 sprite pixel per step
    static constexpr float STEP_INTERVAL  = 1.f / 60.f;   // one step per frame at 60fps
    static constexpr float PLAT_THRESHOLD = 2.f;
    static constexpr float LADR_THRESHOLD = 4.f;          // ±4 px snap zone around each ladder

    void SyncWorldPosition();
};

#pragma once
#include "BaseComponent.h"
#include "LevelMap.h"
#include "LevelLoader.h"
#include <glm/glm.hpp>

class PlatformMovementComponent : public dae::BaseComponent
{
public:
    PlatformMovementComponent(dae::GameObject& owner,
                              const dae::LevelMap* levelMap,
                              glm::vec2 worldPos,
                              float charWorldW, float charWorldH,
                              const LevelTransform& transform);

    void Update() override;
    void Render() override {}

    float GetPosX() const { return m_posX; }
    float GetPosY() const { return m_posY; }

private:
    const dae::LevelMap* m_levelMap;

    float m_posX, m_posY;      // feet-center in world coords
    float m_charHalfW;         // half char width for rendering offset
    float m_charRenderH;       // char height + 2px nudge, for rendering offset
    float m_stepX, m_stepY;    // world pixels per step (= scaleX/Y)
    float m_platThresh;        // y-distance snap for platforms
    float m_ladrThresh;        // x-distance snap for ladders

    float m_stepTimer{ 0.f };

    static constexpr float STEP_INTERVAL = 1.f / 60.f;

    void SyncPosition();
};

#pragma once
#include "BaseComponent.h"
#include "EnemyComponent.h"
#include "PlatformMovementComponent.h"
#include <vector>
#include <glm/glm.hpp>

// Pepper attack: press X to stun nearby enemies in the direction you are facing.
// Alternatives considered:
//   A) PlatformMovementComponent handles pepper directly (simpler but mixes concerns)
//   B) Pepper creates a temporary GameObject hitzone in the scene (cleaner lifecycle, more complex)
//   C) This component owns the pepper state and queries enemies directly (chosen - simple and self-contained)

class PepperComponent : public dae::BaseComponent
{
public:
    PepperComponent(dae::GameObject& owner,
                    PlatformMovementComponent* player,
                    std::vector<EnemyComponent*>* enemies,
                    float charWorldW, float charWorldH,
                    int charges = 3);

    void Update() override;
    void Render() override;

    int GetCharges() const { return m_charges; }
    void AddCharge(int n) { m_charges += n; }

private:
    PlatformMovementComponent* m_player;
    std::vector<EnemyComponent*>* m_enemies;
    float m_charW, m_charH;
    int m_charges;

    bool m_active{ false };
    float m_activeTimer{ 0.f };
    float m_pepperX{}, m_pepperY{}, m_pepperW{}, m_pepperH{};
    bool m_prevKeyDown{ false };

    static constexpr float PEPPER_DURATION = 2.f;
    static constexpr float STUN_DURATION   = 3.f;
    // Pepper cloud: 2 char-widths forward, 1 char-height tall
    static constexpr float PEPPER_FORWARD  = 2.f;
    static constexpr float PEPPER_LATERAL  = 0.5f;
};

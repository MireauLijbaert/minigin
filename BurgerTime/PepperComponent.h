#pragma once
#include "BaseComponent.h"
#include "Subject.h"
#include "Event.h"
#include "GameObject.h"
#include "EnemyComponent.h"
#include "PlatformMovementComponent.h"
#include "Texture2D.h"
#include <functional>
#include <memory>
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
    dae::Subject& GetSubject() { return m_subject; }
    PepperComponent(dae::GameObject& owner,
                    PlatformMovementComponent* player,
                    std::vector<EnemyComponent*>* enemies,
                    float charWorldW, float charWorldH,
                    int charges = 3);

    void Update() override;
    void Render() override;

    int GetCharges() const { return m_charges; }

    void AddCharge(int n)
    {
        m_charges += n;
        NotifyPepperChanged();
    }

    // Called by PlayerAnimatorComponent setup to animate the throw pose
    void SetPepperFiredCallback(std::function<void()> cb) { m_pepperFiredCallback = std::move(cb); }

private:
    PlatformMovementComponent* m_player;
    std::vector<EnemyComponent*>* m_enemies;
    float m_charW, m_charH;
    int m_charges;

    std::function<void()> m_pepperFiredCallback;
    bool m_active{ false };
    float m_activeTimer{ 0.f };
    float m_pepperX{}, m_pepperY{}, m_pepperW{}, m_pepperH{};
    bool m_prevKeyDown{ false };

    // Cloud sprite animation
    std::shared_ptr<dae::Texture2D> m_texH; // sideways, base=RIGHT
    std::shared_ptr<dae::Texture2D> m_texD; // downward
    std::shared_ptr<dae::Texture2D> m_texU; // upward
    glm::vec2 m_cloudDir{ 0.f, 0.f }; // direction captured at throw
    int   m_cloudFrame{ 0 };
    float m_cloudFrameTimer{ 0.f };

    static constexpr int   CLOUD_FRAMES = 4;
    static constexpr float CLOUD_FPS    = 8.f;

    dae::Subject m_subject;

    void NotifyPepperChanged()
    {
        dae::Event event{ "PepperChanged" };
        event.nbArgs = 1;
        event.args[0] = dae::EventArg{ .intValue = m_charges };
        m_subject.NotifyObservers(event, GetOwner());
    }

    static constexpr float PEPPER_DURATION     = 2.f;
    static constexpr float STUN_DURATION       = 3.f;
    static constexpr float THROW_FREEZE_DURATION = 0.5f; // movement halt during throw pose
    // Pepper cloud: 2 char-widths forward, 1 char-height tall
    static constexpr float PEPPER_FORWARD  = 2.f;
    static constexpr float PEPPER_LATERAL  = 0.5f;
};

#include "PepperComponent.h"
#include "Renderer.h"
#include "TimeSingleton.h"
#include "Event.h"
#include <SDL3/SDL.h>
#include <algorithm>

PepperComponent::PepperComponent(dae::GameObject& owner,
                                 PlatformMovementComponent* player,
                                 std::vector<EnemyComponent*>* enemies,
                                 float charWorldW, float charWorldH,
                                 int charges)
    : BaseComponent(owner)
    , m_player{ player }
    , m_enemies{ enemies }
    , m_charW{ charWorldW }
    , m_charH{ charWorldH }
    , m_charges{ charges }
{}

void PepperComponent::Update()
{
    const float dt = dae::Time::GetInstance().GetDeltaTime();

    // Tick active pepper timer
    if (m_active)
    {
        m_activeTimer -= dt;
        if (m_activeTimer <= 0.f)
            m_active = false;
        return;
    }

    if (!m_player->IsAlive() || m_charges <= 0) return;

    const auto* keys = SDL_GetKeyboardState(nullptr);
    const bool keyDown = keys[SDL_SCANCODE_X] != 0;

    // Fire on key down edge (not held)
    if (keyDown && !m_prevKeyDown)
    {
        --m_charges;
        NotifyPepperChanged();

        glm::vec2 dir = m_player->GetFacingDir();
        float px = m_player->GetPosX();
        float py = m_player->GetPosY();

        float halfW = m_charW * PEPPER_FORWARD;
        float halfH = m_charH * PEPPER_LATERAL;

        // Build pepper rect in front of player based on facing direction
        if (dir.x != 0.f)
        {
            float sign = dir.x > 0.f ? 1.f : -1.f;
            m_pepperX = px + sign * m_charW * 0.5f;
            m_pepperY = py - m_charH - halfH;
            m_pepperW = halfW;
            m_pepperH = m_charH + halfH * 2.f;
            if (dir.x < 0.f) m_pepperX -= halfW;
        }
        else
        {
            float sign = dir.y > 0.f ? 1.f : -1.f; // y>0 = up in our convention
            m_pepperX = px - halfH;
            m_pepperW = m_charW + halfH * 2.f;
            m_pepperH = halfW;
            m_pepperY = sign > 0.f ? py - m_charH - halfW : py;
        }

        // Stun enemies inside the pepper rect
        if (m_enemies)
        {
            for (auto* enemy : *m_enemies)
            {
                if (!enemy->IsAlive()) continue;
                float ex = enemy->GetPosX();
                float ey = enemy->GetPosY();
                if (ex >= m_pepperX && ex <= m_pepperX + m_pepperW
                    && ey >= m_pepperY && ey <= m_pepperY + m_pepperH)
                {
                    enemy->Stun();
                }
            }
        }

        m_active = true;
        m_activeTimer = PEPPER_DURATION;
        m_subject.NotifyObservers(dae::Event("PepperFired"), GetOwner());
    }

    m_prevKeyDown = keyDown;
}

void PepperComponent::Render()
{
    if (!m_active) return;

    SDL_Renderer* renderer = dae::Renderer::GetInstance().GetSDLRenderer();
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Flicker: alternate opacity based on remaining timer
    float t = m_activeTimer / PEPPER_DURATION;
    Uint8 alpha = static_cast<Uint8>(120 + 100 * std::sin(m_activeTimer * 20.f));
    SDL_SetRenderDrawColor(renderer, 255, 220, 50, alpha);

    SDL_FRect rect{ m_pepperX, m_pepperY, m_pepperW, m_pepperH };
    SDL_RenderFillRect(renderer, &rect);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    (void)t;
}

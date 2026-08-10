#include "PepperComponent.h"
#include "Renderer.h"
#include "ResourceManager.h"
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
{
    auto& rm = dae::ResourceManager::GetInstance();
    m_texH = rm.LoadTexture("bt_pepper_h.png");
    m_texD = rm.LoadTexture("bt_pepper_d.png");
    m_texU = rm.LoadTexture("bt_pepper_u.png");
}

void PepperComponent::Update()
{
    const float dt = dae::Time::GetInstance().GetDeltaTime();

    // Tick active pepper timer and advance cloud animation
    if (m_active)
    {
        m_activeTimer -= dt;
        if (m_activeTimer <= 0.f)
        {
            m_active = false;
        }
        else
        {
            if (m_cloudFrame < CLOUD_FRAMES) // still animating
            {
                m_cloudFrameTimer += dt;
                if (m_cloudFrameTimer >= 1.f / CLOUD_FPS)
                {
                    m_cloudFrameTimer -= 1.f / CLOUD_FPS;
                    ++m_cloudFrame; // no wrap, stops at CLOUD_FRAMES (done)
                }
            }
        }
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

        m_cloudDir = dir;
        m_cloudFrame = 0;
        m_cloudFrameTimer = 0.f;
        m_active = true;
        m_activeTimer = PEPPER_DURATION;
        m_player->FreezeFor(THROW_FREEZE_DURATION);
        m_subject.NotifyObservers(dae::Event("PepperFired"), GetOwner());
        if (m_pepperFiredCallback) m_pepperFiredCallback();
    }

    m_prevKeyDown = keyDown;
}

void PepperComponent::Render()
{
    if (!m_active) return;

    // Pick the right texture strip based on throw direction
    std::shared_ptr<dae::Texture2D> tex{};
    bool flipH = false;
    if (m_cloudDir.x != 0.f)
    {
        tex   = m_texH;
        flipH = (m_cloudDir.x > 0.f); // base=LEFT, flip for right throw
    }
    else if (m_cloudDir.y > 0.f)
        tex = m_texU;
    else
        tex = m_texD;

    if (!tex || m_cloudFrame >= CLOUD_FRAMES) return; // animation finished
    SDL_Texture* sdlTex = tex->GetSDLTexture();
    if (!sdlTex) return;

    glm::vec2 texSize = tex->GetSize();
    float frameW = texSize.x / static_cast<float>(CLOUD_FRAMES);

    SDL_FRect src{
        static_cast<float>(m_cloudFrame) * frameW,
        0.f, frameW, texSize.y
    };

    // Centre the cloud sprite (charW x charH) within the pepper zone.
    // Vertical throws: subtract charW/2 to centre the sprite on the player's X,
    //   then shift 0.5 charH toward the player so the cloud is adjacent (not 1 charH away).
    float cloudX = m_pepperX + (m_pepperW - m_charW) * 0.5f;
    float cloudY = m_pepperY + (m_pepperH - m_charH) * 0.5f;
    if (m_cloudDir.x == 0.f)
    {
        cloudX -= m_charW * 0.5f;
        // shift toward player: up throw moves cloud down, down throw moves cloud up
        const float sign = m_cloudDir.y > 0.f ? 1.f : -1.f;
        cloudY += sign * m_charH * 0.5f;
    }
    else
    {
        // Pull cloud to player's edge (remove the charW/2 gap)
        const float fwd = m_cloudDir.x > 0.f ? 1.f : -1.f;
        cloudX -= fwd * m_charW * 0.5f;
    }
    SDL_FRect dst{ cloudX, cloudY, m_charW, m_charH };

    SDL_Renderer* renderer = dae::Renderer::GetInstance().GetSDLRenderer();
    if (flipH)
        SDL_RenderTextureRotated(renderer, sdlTex, &src, &dst, 0.0, nullptr, SDL_FLIP_HORIZONTAL);
    else
        SDL_RenderTexture(renderer, sdlTex, &src, &dst);
}

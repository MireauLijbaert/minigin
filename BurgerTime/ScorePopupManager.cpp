#include "ScorePopupManager.h"
#include "ResourceManager.h"
#include "Renderer.h"
#include <SDL3/SDL.h>
#include <algorithm>

int ScorePopupManager::ScoreToFrame(int score)
{
    switch (score)
    {
    case 100:  return 0;
    case 200:  return 1;
    case 300:  return 2;
    case 500:  return 3;
    case 1000: return 4;
    case 2000: return 5;
    case 4000: return 6;
    case 8000: return 7;
    case 1600: return 8;
    default:   return -1;   // unrecognised score, no popup
    }
}

void ScorePopupManager::Spawn(int score, float screenX, float screenY)
{
    int frame = ScoreToFrame(score);
    if (frame < 0) return;

    // Lazy-load the sprite strip.
    if (!m_tex)
        m_tex = dae::ResourceManager::GetInstance().LoadTexture("bt_score_popup.png");

    // Centre the popup on the kill position.
    m_popups.push_back({
        screenX - m_popupW * 0.5f,
        screenY - m_popupH,
        frame,
        POPUP_LIFETIME
    });
}

void ScorePopupManager::Update(float dt)
{
    for (auto& p : m_popups)
        p.life -= dt;
    m_popups.erase(
        std::remove_if(m_popups.begin(), m_popups.end(),
            [](const Popup& p) { return p.life <= 0.f; }),
        m_popups.end());
}

void ScorePopupManager::Render()
{
    if (!m_tex || m_popups.empty()) return;

    SDL_Renderer* renderer = dae::Renderer::GetInstance().GetSDLRenderer();
    SDL_Texture*  sdlTex   = m_tex->GetSDLTexture();
    if (!sdlTex) return;

    const glm::vec2 texSize = m_tex->GetSize();
    const float frameW      = texSize.x / static_cast<float>(POPUP_FRAMES);

    for (const auto& p : m_popups)
    {
        SDL_FRect src{ static_cast<float>(p.frameIdx) * frameW, 0.f, frameW, texSize.y };
        SDL_FRect dst{ p.x, p.y, m_popupW, m_popupH };
        SDL_RenderTexture(renderer, sdlTex, &src, &dst);
    }
}

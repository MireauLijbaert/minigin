#include "AnimatedSpriteComponent.h"
#include "ResourceManager.h"
#include "Renderer.h"
#include "GameObject.h"
#include "TimeSingleton.h"
#include <SDL3/SDL.h>
#include <algorithm>

AnimatedSpriteComponent::AnimatedSpriteComponent(dae::GameObject& owner,
                                                 float renderW, float renderH)
    : BaseComponent(owner)
    , m_renderW{ renderW }
    , m_renderH{ renderH }
{}

void AnimatedSpriteComponent::AddClip(const std::string& name, const std::string& texFile,
                                      int frameCount, float fps, bool loop)
{
    AnimClip clip;
    clip.texture    = dae::ResourceManager::GetInstance().LoadTexture(texFile);
    clip.frameCount = frameCount;
    clip.fps        = fps;
    clip.loop       = loop;
    m_clips[name]   = std::move(clip);

    // Auto-select first registered clip
    if (m_current.empty())
    {
        m_current    = name;
        m_frameIndex = 0;
        m_frameTimer = 0.f;
    }
}

bool AnimatedSpriteComponent::IsClipFinished() const
{
    auto it = m_clips.find(m_current);
    if (it == m_clips.end()) return false;
    const AnimClip& clip = it->second;
    return !clip.loop && (m_frameIndex >= clip.frameCount - 1);
}

void AnimatedSpriteComponent::Play(const std::string& name)
{
    m_paused = false; // switching clips always resumes
    if (name == m_current) return;
    if (m_clips.find(name) == m_clips.end()) return;
    m_current    = name;
    m_frameIndex = 0;
    m_frameTimer = 0.f;
}

void AnimatedSpriteComponent::Update()
{
    if (m_paused) return;
    if (m_current.empty()) return;
    auto it = m_clips.find(m_current);
    if (it == m_clips.end()) return;

    const AnimClip& clip = it->second;
    if (clip.frameCount <= 1) return;

    const float dt = dae::Time::GetInstance().GetDeltaTime();
    m_frameTimer += dt;
    const float frameDur = 1.f / clip.fps;
    while (m_frameTimer >= frameDur)
    {
        m_frameTimer -= frameDur;
        if (clip.loop)
            m_frameIndex = (m_frameIndex + 1) % clip.frameCount;
        else
            m_frameIndex = std::min(m_frameIndex + 1, clip.frameCount - 1);
    }
}

void AnimatedSpriteComponent::Render()
{
    if (m_current.empty()) return;
    auto it = m_clips.find(m_current);
    if (it == m_clips.end()) return;

    const AnimClip& clip = it->second;
    if (!clip.texture) return;

    SDL_Texture* tex = clip.texture->GetSDLTexture();
    if (!tex) return;

    // Source rect: one frame from the horizontal strip
    glm::vec2 texSize = clip.texture->GetSize();
    float frameW = texSize.x / static_cast<float>(clip.frameCount);
    SDL_FRect src{
        static_cast<float>(m_frameIndex) * frameW,
        0.f,
        frameW,
        texSize.y
    };

    // Destination rect: owner world position
    const auto& pos = GetOwner()->GetWorldPosition().GetPosition();
    SDL_FRect dst{ pos.x, pos.y, m_renderW, m_renderH };

    SDL_Renderer* renderer = dae::Renderer::GetInstance().GetSDLRenderer();

    if (m_flipH)
        SDL_RenderTextureRotated(renderer, tex, &src, &dst, 0.0, nullptr, SDL_FLIP_HORIZONTAL);
    else
        SDL_RenderTexture(renderer, tex, &src, &dst);
}

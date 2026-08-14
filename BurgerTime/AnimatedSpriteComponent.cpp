#include "AnimatedSpriteComponent.h"
#include "ResourceManager.h"
#include "TimeSingleton.h"
#include <algorithm>

AnimatedSpriteComponent::AnimatedSpriteComponent(dae::GameObject& owner,
                                                 dae::RenderComponent* renderComp)
    : BaseComponent(owner)
    , m_renderComp{ renderComp }
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
        SyncRenderComp();
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
    m_paused = false;
    if (name == m_current) return;
    if (m_clips.find(name) == m_clips.end()) return;
    m_current    = name;
    m_frameIndex = 0;
    m_frameTimer = 0.f;
    SyncRenderComp();
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
    bool advanced = false;
    while (m_frameTimer >= frameDur)
    {
        m_frameTimer -= frameDur;
        if (clip.loop)
            m_frameIndex = (m_frameIndex + 1) % clip.frameCount;
        else
            m_frameIndex = std::min(m_frameIndex + 1, clip.frameCount - 1);
        advanced = true;
    }
    if (advanced) SyncRenderComp();
}

void AnimatedSpriteComponent::SyncRenderComp()
{
    if (!m_renderComp) return;
    auto it = m_clips.find(m_current);
    if (it == m_clips.end()) return;

    const AnimClip& clip = it->second;
    if (!clip.texture) return;

    m_renderComp->SetTexture(clip.texture);

    const float totalW = clip.texture->GetSize().x;
    const float frameW = totalW / static_cast<float>(clip.frameCount);
    const float frameH = clip.texture->GetSize().y;
    m_renderComp->SetSourceRect(
        static_cast<float>(m_frameIndex) * frameW, 0.f,
        frameW, frameH);
}

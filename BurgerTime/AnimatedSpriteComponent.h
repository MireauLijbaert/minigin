#pragma once
#include "BaseComponent.h"
#include "RenderComponent.h"
#include "Texture2D.h"
#include <memory>
#include <string>
#include <unordered_map>

struct AnimClip
{
    std::shared_ptr<dae::Texture2D> texture; // horizontal strip: each frame is (stripW/frameCount) wide
    int   frameCount{ 1 };
    float fps{ 8.f };
    bool  loop{ true };
};

// Drives a RenderComponent with per-frame source rects from a sprite strip.
// The RenderComponent owns the draw call; this component owns the animation logic.
class AnimatedSpriteComponent : public dae::BaseComponent
{
public:
    // renderComp must already be added to the same GameObject with SetSize() called.
    AnimatedSpriteComponent(dae::GameObject& owner, dae::RenderComponent* renderComp);

    // Register a clip; texFile is a horizontal strip with frameCount equally-wide frames.
    void AddClip(const std::string& name, const std::string& texFile,
                 int frameCount, float fps, bool loop = true);

    // Switch to a named clip (resets frame only if the clip actually changes).
    // Also clears any pause set by Pause().
    void Play(const std::string& name);

    // Freeze the current frame in place. Cleared automatically by Play().
    void Pause()  { m_paused = true; }
    void Resume() { m_paused = false; }

    // Flip the rendered sprite horizontally — forwarded to the RenderComponent.
    void SetFlipH(bool flip) { if (m_renderComp) m_renderComp->SetFlipH(flip); }

    // Optional RGB tint — forwarded to the RenderComponent.
    void SetColorMod(uint8_t r, uint8_t g, uint8_t b)
    {
        if (m_renderComp) m_renderComp->SetColorMod(r, g, b);
    }

    // True when the current clip is non-looping and has reached its last frame.
    bool IsClipFinished() const;

    void Update() override;
    void Render() override {} // RenderComponent handles drawing

private:
    dae::RenderComponent* m_renderComp;
    std::unordered_map<std::string, AnimClip> m_clips;
    std::string m_current;
    int   m_frameIndex{ 0 };
    float m_frameTimer{ 0.f };
    bool  m_paused{ false };

    // Push the current clip/frame into the RenderComponent.
    void SyncRenderComp();
};

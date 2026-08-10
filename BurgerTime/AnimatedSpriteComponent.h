#pragma once
#include "BaseComponent.h"
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

// Renders one frame at a time from a horizontal sprite strip.
// Supports optional horizontal flip for left-facing sprites.
class AnimatedSpriteComponent : public dae::BaseComponent
{
public:
    AnimatedSpriteComponent(dae::GameObject& owner, float renderW, float renderH);

    // Register a clip; texFile is a horizontal strip with frameCount equally-wide frames.
    void AddClip(const std::string& name, const std::string& texFile,
                 int frameCount, float fps, bool loop = true);

    // Switch to a named clip (resets frame only if the clip actually changes).
    // Also clears any pause set by Pause().
    void Play(const std::string& name);

    // Freeze the current frame in place. Cleared automatically by Play().
    void Pause()  { m_paused = true; }
    void Resume() { m_paused = false; }

    // Flip the rendered sprite horizontally (for left-facing variants).
    void SetFlipH(bool flip) { m_flipH = flip; }

    // True when the current clip is non-looping and has reached its last frame.
    bool IsClipFinished() const;

    void Update() override;
    void Render() override;

private:
    std::unordered_map<std::string, AnimClip> m_clips;
    std::string m_current;
    int   m_frameIndex{ 0 };
    float m_frameTimer{ 0.f };
    float m_renderW, m_renderH;
    bool  m_flipH{ false };
    bool  m_paused{ false };
};

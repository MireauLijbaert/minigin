#pragma once
#include "BaseComponent.h"
#include "Observer.h"
#include "Subject.h"
#include "Event.h"
#include "TextComponent.h"
#include "HealthComponent.h"
#include "PepperComponent.h"
#include "GameObject.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "TimeSingleton.h"
#include "Texture2D.h"
#include "ScorePopupManager.h"
#include "ServiceLocator.h"
#include "SoundSystem.h"
#include "InputManager.h"
#include "Command.h"
#include <SDL3/SDL.h>
#include <memory>
#include <string>

// ---- Score singleton --------------------------------------------------------

class ScoreManager
{
public:
    static ScoreManager& GetInstance()
    {
        static ScoreManager instance;
        return instance;
    }

    void AddScore(int amount)
    {
        m_score += amount;
        if (m_score > m_hiScore)
        {
            m_hiScore = m_score;
            NotifyHiScoreChanged();
        }
        NotifyScoreChanged();
    }

    int  GetScore()   const { return m_score; }
    int  GetHiScore() const { return m_hiScore; }

    void Reset()
    {
        m_score = 0;
        NotifyScoreChanged();
        // hi-score intentionally survives Reset() persists across games
    }

    // Called once at startup: ensures the in-memory hi-score is at least
    // as high as the top entry loaded from disk.
    void SetHiScoreFloor(int floor)
    {
        if (floor > m_hiScore)
        {
            m_hiScore = floor;
            NotifyHiScoreChanged();
        }
    }

    dae::Subject& GetSubject()        { return m_subject; }
    dae::Subject& GetHiScoreSubject() { return m_hiSubject; }

private:
    ScoreManager() = default;
    int          m_score  { 0 };
    int          m_hiScore{ 0 };
    dae::Subject m_subject;
    dae::Subject m_hiSubject;

    void NotifyScoreChanged()
    {
        dae::Event event{ "ScoreChanged" };
        event.nbArgs  = 1;
        event.args[0] = dae::EventArg{ .intValue = m_score };
        m_subject.NotifyObservers(event, nullptr);
    }

    void NotifyHiScoreChanged()
    {
        dae::Event event{ "HiScoreChanged" };
        event.nbArgs  = 1;
        event.args[0] = dae::EventArg{ .intValue = m_hiScore };
        m_hiSubject.NotifyObservers(event, nullptr);
    }
};

// ---- Text-based display components (Observer-based) -------------------------
// These render via SDL_ttf TextComponent; used for score and pepper count.

class ScoreDisplayComponent : public dae::BaseComponent, public dae::Observer
{
public:
    ScoreDisplayComponent(dae::GameObject& owner, dae::TextComponent* text)
        : BaseComponent(owner)
        , m_text{ text }
        , m_subject{ &ScoreManager::GetInstance().GetSubject() }
    {
        m_text->SetText(std::to_string(ScoreManager::GetInstance().GetScore()));
        m_subject->AddObserver(this);
    }

    ~ScoreDisplayComponent() override
    {
        if (m_subject) m_subject->RemoveObserver(this);
    }

    void OnSubjectDestroyed(dae::Subject* subject) override
    {
        if (subject == m_subject) m_subject = nullptr;
    }

    void Notify(const dae::Event& event, dae::GameObject*) override
    {
        if (event.id == "ScoreChanged")
            m_text->SetText(std::to_string(event.args[0].intValue));
    }

    void Update() override {}
    void Render() override {}

private:
    dae::TextComponent* m_text;
    dae::Subject*       m_subject;
};

class HiScoreDisplayComponent : public dae::BaseComponent, public dae::Observer
{
public:
    HiScoreDisplayComponent(dae::GameObject& owner, dae::TextComponent* text)
        : BaseComponent(owner)
        , m_text{ text }
        , m_subject{ &ScoreManager::GetInstance().GetHiScoreSubject() }
    {
        m_text->SetText(std::to_string(ScoreManager::GetInstance().GetHiScore()));
        m_subject->AddObserver(this);
    }

    ~HiScoreDisplayComponent() override
    {
        if (m_subject) m_subject->RemoveObserver(this);
    }

    void OnSubjectDestroyed(dae::Subject* subject) override
    {
        if (subject == m_subject) m_subject = nullptr;
    }

    void Notify(const dae::Event& event, dae::GameObject*) override
    {
        if (event.id == "HiScoreChanged")
            m_text->SetText(std::to_string(event.args[0].intValue));
    }

    void Update() override {}
    void Render() override {}

private:
    dae::TextComponent* m_text;
    dae::Subject*       m_subject;
};

class PepperDisplayComponent : public dae::BaseComponent, public dae::Observer
{
public:
    PepperDisplayComponent(dae::GameObject& owner, dae::TextComponent* text,
                           PepperComponent* pepper)
        : BaseComponent(owner)
        , m_text{ text }
        , m_subject{ &pepper->GetSubject() }
    {
        m_text->SetText(std::to_string(pepper->GetCharges()));
        m_subject->AddObserver(this);
    }

    ~PepperDisplayComponent() override
    {
        if (m_subject) m_subject->RemoveObserver(this);
    }

    void OnSubjectDestroyed(dae::Subject* subject) override
    {
        if (subject == m_subject) m_subject = nullptr;
    }

    void Notify(const dae::Event& event, dae::GameObject*) override
    {
        if (event.id == "PepperChanged")
            m_text->SetText(std::to_string(event.args[0].intValue));
    }

    void Update() override {}
    void Render() override {}

private:
    dae::TextComponent* m_text;
    dae::Subject*       m_subject;
};

// ---- Sprite-based lives display (Observer-based) ----------------------------
// Renders a row of player-head sprite icons, one per remaining life.

class LivesSpriteComponent : public dae::BaseComponent, public dae::Observer
{
public:
    LivesSpriteComponent(dae::GameObject& owner,
                         dae::HealthComponent* health,
                         float iconW, float iconH,
                         float posX, float posY)
        : BaseComponent(owner)
        , m_iconW{ iconW }, m_iconH{ iconH }
        , m_posX{ posX }, m_posY{ posY }
        , m_lives{ health->GetLives() }
        , m_subject{ &health->GetSubject() }
    {
        m_subject->AddObserver(this);
        m_tex = dae::ResourceManager::GetInstance().LoadTexture("bt_hud_life.png");
    }

    ~LivesSpriteComponent() override
    {
        if (m_subject) m_subject->RemoveObserver(this);
    }

    void OnSubjectDestroyed(dae::Subject* s) override
    {
        if (s == m_subject) m_subject = nullptr;
    }

    void Notify(const dae::Event& event, dae::GameObject*) override
    {
        if (event.id == "LifeChanged")
            m_lives = event.args[0].intValue;
    }

    void Update() override {}

    void Render() override
    {
        if (!m_tex || m_lives <= 0) return;
        SDL_Renderer* renderer = dae::Renderer::GetInstance().GetSDLRenderer();
        SDL_Texture*  sdlTex   = m_tex->GetSDLTexture();
        if (!sdlTex) return;

        constexpr float GAP = 4.f;
        for (int i = 0; i < m_lives; ++i)
        {
            SDL_FRect dst{ m_posX + static_cast<float>(i) * (m_iconW + GAP),
                           m_posY, m_iconW, m_iconH };
            SDL_RenderTexture(renderer, sdlTex, nullptr, &dst);
        }
    }

private:
    std::shared_ptr<dae::Texture2D> m_tex;
    float       m_iconW, m_iconH;
    float       m_posX, m_posY;
    int         m_lives;
    dae::Subject* m_subject;
};

// ---- ScorePopupComponent ----------------------------------------------------
// Thin wrapper: just calls ScorePopupManager each frame.
// Place on any persistent HUD GameObject in the scene.

class ScorePopupComponent : public dae::BaseComponent
{
public:
    explicit ScorePopupComponent(dae::GameObject& owner)
        : BaseComponent(owner) {}

    void Update() override
    {
        ScorePopupManager::GetInstance().Update(
            dae::Time::GetInstance().GetDeltaTime());
    }

    void Render() override
    {
        ScorePopupManager::GetInstance().Render();
    }
};

// ---- MuteToggleComponent ----------------------------------------------------
// F2 flips global mute (event-based via InputManager, no polling, no hold-repeat).

class MuteToggleComponent : public dae::BaseComponent
{
public:
    MuteToggleComponent(dae::GameObject& owner, dae::TextComponent* label = nullptr)
        : BaseComponent(owner), m_label{ label }
    {
        dae::InputManager::GetInstance().BindKeyboardInput(
            SDL_SCANCODE_F2,
            std::make_unique<dae::LambdaCommand>([this]()
            {
                auto& ss = dae::ServiceLocator::GetSoundSystem();
                const bool nowMuted = !ss.IsMuted();
                ss.SetMuted(nowMuted);
                if (m_label)
                    m_label->SetText(nowMuted ? "[F2] MUTED" : "[F2] SOUND");
            }),
            dae::InputState::Down
        );
    }

    void Update() override {}
    void Render() override {}

private:
    dae::TextComponent* m_label;
};

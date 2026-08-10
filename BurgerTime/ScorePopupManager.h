#pragma once
#include "Texture2D.h"
#include <memory>
#include <vector>

class ScorePopupManager
{
public:
    static ScorePopupManager& GetInstance()
    {
        static ScorePopupManager inst;
        return inst;
    }

    // Configure how large each popup is rendered (match character scale).
    void SetDisplaySize(float w, float h) { m_popupW = w; m_popupH = h; }

    // Spawn a score popup centred at (screenX, screenY).
    void Spawn(int score, float screenX, float screenY);

    void Update(float dt);
    void Render();
    void Reset() { m_popups.clear(); }

private:
    ScorePopupManager() = default;

    struct Popup
    {
        float x, y;      // top-left screen position
        int   frameIdx;  // which sprite in the strip (see ScoreToFrame)
        float life;      // seconds remaining
    };

    std::shared_ptr<dae::Texture2D> m_tex;
    std::vector<Popup>              m_popups;
    float m_popupW{ 48.f };
    float m_popupH{ 24.f };

    static constexpr float POPUP_LIFETIME  = 1.0f;
    static constexpr int   POPUP_FRAMES    = 9;      // frames in the strip

    // Maps a score value to its column index in bt_score_popup.png:
    //   col 0=100, 1=200, 2=300, 3=500, 4=1000, 5=2000, 6=4000, 7=8000, 8=1600
    static int ScoreToFrame(int score);
};

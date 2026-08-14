#pragma once
#include "BaseComponent.h"
#include "Font.h"
#include "InputManager.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Texture2D.h"
#include "HighScoreManager.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <vector>

// Arcade-style name entry.
//
// Top half:  letter grid (A-Z + symbols + SPA / RUB / END)
//            Peter Pepper sprite points at the selected cell.
// Bottom half: NAME / SCORE table showing existing top-5 with the
//              current player's live entry inserted at their rank.
//
// D-pad / arrow keys navigate; A / Enter selects.

class NameEntryComponent : public dae::BaseComponent
{
public:
    NameEntryComponent(dae::GameObject& owner,
                       std::shared_ptr<dae::Font> gridFont,
                       std::shared_ptr<dae::Font> tableFont,
                       int finalScore,
                       std::vector<HighScoreEntry> existingEntries,
                       std::function<void(const std::string&)> onConfirm)
        : BaseComponent(owner)
        , m_gridFont{ std::move(gridFont) }
        , m_tableFont{ std::move(tableFont) }
        , m_finalScore{ finalScore }
        , m_existingEntries{ std::move(existingEntries) }
        , m_onConfirm{ std::move(onConfirm) }
    {
        m_playerSprite = dae::ResourceManager::GetInstance().LoadTexture("bt_player_idle.png");
    }

    void Update() override
    {
        if (m_done) return;
        const auto* keys = SDL_GetKeyboardState(nullptr);
        auto& input = dae::InputManager::GetInstance();

        auto edge = [&](bool down, bool& prev) -> bool
        {
            bool hit = down && !prev;
            prev = down;
            return hit;
        };

        if (edge(keys[SDL_SCANCODE_LEFT]  || keys[SDL_SCANCODE_A] || input.IsGamepadButtonHeld(dae::GamepadButton::DPadLeft,  0), m_prevL)) MoveLeft();
        if (edge(keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D] || input.IsGamepadButtonHeld(dae::GamepadButton::DPadRight, 0), m_prevR)) MoveRight();
        if (edge(keys[SDL_SCANCODE_UP]    || keys[SDL_SCANCODE_W] || input.IsGamepadButtonHeld(dae::GamepadButton::DPadUp,    0), m_prevU)) MoveUp();
        if (edge(keys[SDL_SCANCODE_DOWN]  || keys[SDL_SCANCODE_S] || input.IsGamepadButtonHeld(dae::GamepadButton::DPadDown,  0), m_prevD)) MoveDown();

        bool selDown = keys[SDL_SCANCODE_RETURN] != 0
                    || keys[SDL_SCANCODE_SPACE]  != 0
                    || input.IsGamepadButtonHeld(dae::GamepadButton::A, 0);
        if (selDown && !m_prevSel) SelectCurrent();
        m_prevSel = selDown;
    }

    void Render() override
    {
        SDL_Renderer* r     = dae::Renderer::GetInstance().GetSDLRenderer();
        TTF_Font*     gFont = m_gridFont->GetFont();
        TTF_Font*     tFont = m_tableFont->GetFont();

        // ── Letter grid ──────────────────────────────────────────────────
        for (int row = 0; row < ROWS; ++row)
        {
            int ncols = ColCount(row);
            for (int col = 0; col < ncols; ++col)
            {
                SDL_FRect cell = CellRect(row, col);
                bool isCursor  = (m_cursorRow == row && m_cursorCol == col);

                SDL_Color fg = isCursor
                    ? SDL_Color{ 255, 50, 50, 255 }   // red when selected
                    : SDL_Color{ 255, 255, 255, 255 }; // white otherwise
                DrawTextCentered(r, gFont, GRID[row][col],
                                 cell.x + cell.w * 0.5f,
                                 cell.y + cell.h * 0.5f, fg);
            }
        }

        // ── Peter Pepper sprite as cursor indicator ───────────────────────
        if (m_playerSprite)
        {
            SDL_FRect cell  = CellRect(m_cursorRow, m_cursorCol);
            constexpr float SPR = 40.f;
            SDL_FRect dst{
                cell.x + cell.w,           // just to the right of selected cell
                cell.y + (cell.h - SPR) * 0.5f,
                SPR, SPR
            };
            SDL_RenderTexture(r, m_playerSprite->GetSDLTexture(), nullptr, &dst);
        }

        // ── Current name being typed ──────────────────────────────────────
        {
            // Build display string "_ _ _" with typed chars filled in
            char buf[8];
            buf[0] = m_nameLen > 0 ? m_name[0] : '_';
            buf[1] = ' ';
            buf[2] = m_nameLen > 1 ? m_name[1] : '_';
            buf[3] = ' ';
            buf[4] = m_nameLen > 2 ? m_name[2] : '_';
            buf[5] = '\0';
            DrawTextCentered(r, tFont, buf, 512.f, GRID_Y - 28.f, { 255, 220, 0, 255 });
        }

        // ── Score table ───────────────────────────────────────────────────
        constexpr float TBL_Y    = TABLE_Y;
        constexpr float ROW_H    = 32.f;
        constexpr float RANK_X   = 200.f;
        constexpr float NAME_X   = 340.f;
        constexpr float SCORE_X  = 560.f;

        // Headers
        DrawTextAt(r, tFont, "NAME",  NAME_X,  TBL_Y, { 220, 50, 50, 255 });
        DrawTextAt(r, tFont, "SCORE", SCORE_X, TBL_Y, { 220, 50, 50, 255 });

        // Build display list: insert current player at correct rank
        struct Row { std::string name; int score; bool isCurrent; };
        std::vector<Row> rows;
        bool inserted = false;
        for (const auto& e : m_existingEntries)
        {
            if (!inserted && m_finalScore >= e.score && rows.size() < 5)
            {
                rows.push_back({ CurrentNameStr(), m_finalScore, true });
                inserted = true;
            }
            if (rows.size() < 5)
                rows.push_back({ e.name, e.score, false });
        }
        if (!inserted && rows.size() < 5)
            rows.push_back({ CurrentNameStr(), m_finalScore, true });
        while (rows.size() < 5)
            rows.push_back({ "---", 0, false });

        for (int i = 0; i < 5; ++i)
        {
            float ry = TBL_Y + ROW_H + i * ROW_H;
            SDL_Color rankCol = { 220, 50, 50, 255 };
            SDL_Color dataCol = rows[i].isCurrent
                ? SDL_Color{ 255, 220, 0, 255 }   // yellow for current player
                : SDL_Color{ 255, 255, 255, 255 };

            char rankBuf[4];
            std::snprintf(rankBuf, sizeof(rankBuf), "%d", i + 1);
            DrawTextAt(r, tFont, rankBuf, RANK_X, ry, rankCol);

            if (rows[i].score > 0)
            {
                DrawTextAt(r, tFont, rows[i].name.c_str(), NAME_X, ry, dataCol);
                char scoreBuf[16];
                std::snprintf(scoreBuf, sizeof(scoreBuf), "%d PTS", rows[i].score);
                DrawTextAt(r, tFont, scoreBuf, SCORE_X, ry, dataCol);
            }
            else
            {
                DrawTextAt(r, tFont, "---", NAME_X, ry, dataCol);
            }
        }
    }

private:
    // ── Grid definition ──────────────────────────────────────────────────────
    static constexpr int ROWS = 4;
    static constexpr const char* GRID[4][10]
    {
        { "A","B","C","D","E","F","G","H","I","J" },
        { "K","L","M","N","O","P","Q","R","S","T" },
        { "U","V","W","X","Y","Z","-",",","?","!" },
        { "SPA","RUB","END",nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr }
    };
    static constexpr int ColCount(int row) { return (row < 3) ? 10 : 3; }

    // ── Layout ───────────────────────────────────────────────────────────────
    static constexpr float GRID_X  = 162.f;  // left edge (10 cols × 70px = 700px, centred in 1024)
    static constexpr float GRID_Y  = 115.f;  // top of grid (below "GAME OVER" header)
    static constexpr float CELL_W  = 70.f;
    static constexpr float CELL_H  = 46.f;
    static constexpr float TABLE_Y = 345.f;  // top of score table

    SDL_FRect CellRect(int row, int col) const
    {
        if (row < 3)
            return { GRID_X + col * CELL_W, GRID_Y + row * CELL_H, CELL_W, CELL_H };
        // Action row: 3 equal-width items spanning full grid
        constexpr float WIDE = CELL_W * 10.f / 3.f;
        return { GRID_X + col * WIDE, GRID_Y + 3 * CELL_H + 8.f, WIDE, CELL_H };
    }

    // ── State ────────────────────────────────────────────────────────────────
    std::shared_ptr<dae::Font>               m_gridFont;
    std::shared_ptr<dae::Font>               m_tableFont;
    std::shared_ptr<dae::Texture2D>          m_playerSprite;
    int                                      m_finalScore;
    std::vector<HighScoreEntry>              m_existingEntries;
    std::function<void(const std::string&)>  m_onConfirm;

    int  m_cursorRow{ 0 };
    int  m_cursorCol{ 0 };
    char m_name[3]{ '_','_','_' };
    int  m_nameLen{ 0 };
    bool m_done{ false };

    bool m_prevL{ false }, m_prevR{ false };
    bool m_prevU{ false }, m_prevD{ false };
    bool m_prevSel{ false };

    // ── Helpers ──────────────────────────────────────────────────────────────
    std::string CurrentNameStr() const
    {
        std::string s;
        for (int i = 0; i < 3; ++i)
            s += (i < m_nameLen) ? m_name[i] : '_';
        return s;
    }

    // ── Navigation ───────────────────────────────────────────────────────────
    void MoveLeft()  { m_cursorCol = (m_cursorCol - 1 + ColCount(m_cursorRow)) % ColCount(m_cursorRow); }
    void MoveRight() { m_cursorCol = (m_cursorCol + 1) % ColCount(m_cursorRow); }
    void MoveUp()
    {
        if (m_cursorRow > 0)
        {
            --m_cursorRow;
            m_cursorCol = std::min(m_cursorCol, ColCount(m_cursorRow) - 1);
        }
    }
    void MoveDown()
    {
        if (m_cursorRow < ROWS - 1)
        {
            ++m_cursorRow;
            m_cursorCol = std::min(m_cursorCol, ColCount(m_cursorRow) - 1);
        }
    }

    // ── Selection ────────────────────────────────────────────────────────────
    void SelectCurrent()
    {
        if (m_cursorRow < 3)
        {
            if (m_nameLen < 3)
                m_name[m_nameLen++] = GRID[m_cursorRow][m_cursorCol][0];
        }
        else
        {
            switch (m_cursorCol)
            {
            case 0: if (m_nameLen < 3) m_name[m_nameLen++] = '_'; break;  // SPA
            case 1: if (m_nameLen > 0) m_name[--m_nameLen] = '_'; break;  // RUB
            case 2:                                                         // END
                m_done = true;
                m_onConfirm(std::string(m_name, 3));
                break;
            }
        }
    }

    // ── Render helpers ───────────────────────────────────────────────────────
    static void DrawTextCentered(SDL_Renderer* r, TTF_Font* font,
                                 const char* text, float cx, float cy, SDL_Color col)
    {
        SDL_Surface* s = TTF_RenderText_Blended(font, text, SDL_strlen(text), col);
        if (!s) return;
        SDL_Texture* t = SDL_CreateTextureFromSurface(r, s);
        if (t)
        {
            SDL_FRect dst{ cx - s->w * 0.5f, cy - s->h * 0.5f,
                           static_cast<float>(s->w), static_cast<float>(s->h) };
            SDL_RenderTexture(r, t, nullptr, &dst);
            SDL_DestroyTexture(t);
        }
        SDL_DestroySurface(s);
    }

    static void DrawTextAt(SDL_Renderer* r, TTF_Font* font,
                           const char* text, float x, float y, SDL_Color col)
    {
        SDL_Surface* s = TTF_RenderText_Blended(font, text, SDL_strlen(text), col);
        if (!s) return;
        SDL_Texture* t = SDL_CreateTextureFromSurface(r, s);
        if (t)
        {
            SDL_FRect dst{ x, y, static_cast<float>(s->w), static_cast<float>(s->h) };
            SDL_RenderTexture(r, t, nullptr, &dst);
            SDL_DestroyTexture(t);
        }
        SDL_DestroySurface(s);
    }
};

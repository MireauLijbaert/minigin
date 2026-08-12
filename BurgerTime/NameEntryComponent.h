#pragma once
#include "BaseComponent.h"
#include "Font.h"
#include "Renderer.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <algorithm>
#include <functional>
#include <memory>
#include <string>

// Cursor-based 3-letter name entry controller/gamepad compatible.
//
// Layout matches BurgerTime arcade:
//   Row 0-2  : letter grid  (A-Z + symbols)
//   Row 3    : SPA | RUB | END
//
// Arrow keys / D-pad navigate; Enter / Space / confirm-button selects.
// SPA inserts '_', RUB erases, END confirms (name is always exactly 3 chars,
// unset slots become '_').

class NameEntryComponent : public dae::BaseComponent
{
public:
    // gridFont  : font used for the letter grid labels
    // nameFont  : (larger) font used for the 3-char name display at top
    // onConfirm : called with the 3-char name when the player selects END
    NameEntryComponent(dae::GameObject& owner,
                       std::shared_ptr<dae::Font> gridFont,
                       std::shared_ptr<dae::Font> nameFont,
                       std::function<void(const std::string&)> onConfirm)
        : BaseComponent(owner)
        , m_gridFont{ std::move(gridFont) }
        , m_nameFont{ std::move(nameFont) }
        , m_onConfirm{ std::move(onConfirm) }
    {}

    // ── Input ────────────────────────────────────────────────────────────────
    void Update() override
    {
        if (m_done) return;
        const auto* keys = SDL_GetKeyboardState(nullptr);

        auto edge = [&](SDL_Scancode sc, bool& prev) -> bool
        {
            bool down = keys[sc] != 0;
            bool hit  = down && !prev;
            prev = down;
            return hit;
        };

        if (edge(SDL_SCANCODE_LEFT,  m_prevL)) MoveLeft();
        if (edge(SDL_SCANCODE_RIGHT, m_prevR)) MoveRight();
        if (edge(SDL_SCANCODE_UP,    m_prevU)) MoveUp();
        if (edge(SDL_SCANCODE_DOWN,  m_prevD)) MoveDown();

        // Select: Enter or Space
        bool selectDown = keys[SDL_SCANCODE_RETURN] != 0 || keys[SDL_SCANCODE_SPACE] != 0;
        if (selectDown && !m_prevSel) SelectCurrent();
        m_prevSel = selectDown;
    }

    // ── Rendering ────────────────────────────────────────────────────────────
    void Render() override
    {
        SDL_Renderer* r  = dae::Renderer::GetInstance().GetSDLRenderer();
        TTF_Font* gFont  = m_gridFont->GetFont();
        TTF_Font* nFont  = m_nameFont->GetFont();

        // ── Name slots (top centre) ──────────────────────────────────────
        constexpr float SLOT  = 52.f;
        constexpr float GAP   = 18.f;
        constexpr float TOTAL = 3 * SLOT + 2 * GAP;
        float slotX0 = (1024.f - TOTAL) * 0.5f;
        float slotY  = 90.f;

        for (int i = 0; i < 3; ++i)
        {
            float sx = slotX0 + i * (SLOT + GAP);
            SDL_FRect box{ sx, slotY, SLOT, SLOT };

            // Slot outline
            SDL_SetRenderDrawColor(r, 200, 200, 200, 255);
            SDL_RenderRect(r, &box);

            if (m_name[i] != '_')
            {
                char buf[2]{ m_name[i], '\0' };
                DrawTextCentered(r, nFont, buf, sx + SLOT * 0.5f, slotY + SLOT * 0.5f,
                                 { 255, 255, 0, 255 });
            }
        }

        // ── Instruction hint ─────────────────────────────────────────────
        DrawTextAt(r, gFont, "ARROWS=MOVE   ENTER=SELECT",
                   262.f, 158.f, { 150, 150, 150, 255 });

        // ── Letter grid ──────────────────────────────────────────────────
        for (int row = 0; row < ROWS; ++row)
        {
            int ncols = ColCount(row);
            for (int col = 0; col < ncols; ++col)
            {
                bool cursor = (m_cursorRow == row && m_cursorCol == col);
                SDL_FRect cell = CellRect(row, col);

                // Cursor highlight
                if (cursor)
                {
                    SDL_SetRenderDrawColor(r, 180, 0, 0, 255);
                    SDL_RenderFillRect(r, &cell);
                }

                SDL_Color fg = cursor ? SDL_Color{ 255, 255, 0, 255 }
                                      : SDL_Color{ 255, 255, 255, 255 };
                DrawTextCentered(r, gFont, GRID[row][col],
                                 cell.x + cell.w * 0.5f,
                                 cell.y + cell.h * 0.5f,
                                 fg);
            }
        }
    }

private:
    // ── Grid definition ──────────────────────────────────────────────────────
    static constexpr int ROWS = 4;

    // Row 0-2: 10 cells each; Row 3: 3 action cells (SPA / RUB / END)
    static constexpr const char* GRID[4][10]
    {
        { "A","B","C","D","E","F","G","H","I","J" },
        { "K","L","M","N","O","P","Q","R","S","T" },
        { "U","V","W","X","Y","Z","-",",","?","!" },
        { "SPA","RUB","END", nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr }
    };

    static constexpr int ColCount(int row) { return (row < 3) ? 10 : 3; }

    // ── Layout ───────────────────────────────────────────────────────────────
    static constexpr float GRID_X  = 262.f;   // left edge of letter grid
    static constexpr float GRID_Y  = 178.f;   // top edge of letter grid
    static constexpr float CELL_W  = 50.f;    // cell width  (10 cols → 500px)
    static constexpr float CELL_H  = 52.f;    // cell height

    SDL_FRect CellRect(int row, int col) const
    {
        if (row < 3)
        {
            return { GRID_X + col * CELL_W,
                     GRID_Y + row * CELL_H,
                     CELL_W, CELL_H };
        }
        // Action row: 3 items spanning the full grid width (500px)
        constexpr float WIDE = CELL_W * 10.f / 3.f;
        return { GRID_X + col * WIDE,
                 GRID_Y + 3 * CELL_H + 6.f,
                 WIDE, CELL_H };
    }

    // ── State ────────────────────────────────────────────────────────────────
    std::shared_ptr<dae::Font>              m_gridFont;
    std::shared_ptr<dae::Font>              m_nameFont;
    std::function<void(const std::string&)> m_onConfirm;

    int  m_cursorRow{ 0 };
    int  m_cursorCol{ 0 };
    char m_name[3]{ '_', '_', '_' };
    int  m_nameLen{ 0 };
    bool m_done{ false };

    bool m_prevL{ false }, m_prevR{ false };
    bool m_prevU{ false }, m_prevD{ false };
    bool m_prevSel{ false };

    // ── Navigation ───────────────────────────────────────────────────────────
    void MoveLeft()
    {
        m_cursorCol = (m_cursorCol - 1 + ColCount(m_cursorRow)) % ColCount(m_cursorRow);
    }
    void MoveRight()
    {
        m_cursorCol = (m_cursorCol + 1) % ColCount(m_cursorRow);
    }
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
            // Regular letter / symbol, take first char of label
            if (m_nameLen < 3)
                m_name[m_nameLen++] = GRID[m_cursorRow][m_cursorCol][0];
        }
        else
        {
            switch (m_cursorCol)
            {
            case 0: // SPA: underscore (space would break file parsing)
                if (m_nameLen < 3) m_name[m_nameLen++] = '_';
                break;
            case 1: // RUB: backspace
                if (m_nameLen > 0) m_name[--m_nameLen] = '_';
                break;
            case 2: // END: confirm (pad remaining slots with '_')
                m_done = true;
                m_onConfirm(std::string(m_name, 3));
                break;
            }
        }
    }

    // ── Render helpers ───────────────────────────────────────────────────────
    static void DrawTextCentered(SDL_Renderer* r, TTF_Font* font,
                                 const char* text,
                                 float cx, float cy, SDL_Color color)
    {
        SDL_Surface* s = TTF_RenderText_Blended(font, text, SDL_strlen(text), color);
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
                           const char* text, float x, float y, SDL_Color color)
    {
        SDL_Surface* s = TTF_RenderText_Blended(font, text, SDL_strlen(text), color);
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

#include "BurgerPieceComponent.h"
#include "PlatformMovementComponent.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "TimeSingleton.h"
#include <SDL3/SDL.h>
#include <cmath>

// Float positions based on the row and column of the piece in the level map
static constexpr int platformY(int row) { return 1 + row * 16; }
static constexpr int ladderX(int col) { return 8 + col * 24; }

BurgerPieceComponent::BurgerPieceComponent(dae::GameObject& owner,
                                            const dae::LevelMap* levelMap,
                                            BurgerType type,
                                            int platformRow,
                                            int startCol,
                                            float scaleX, float scaleY,
                                            float offsetX, float offsetY,
                                            PlatformMovementComponent* player,
                                            std::vector<BurgerPieceComponent*>* allPieces,
                                            const std::vector<CupDef>* cups)
    : BaseComponent(owner)
    , m_levelMap{ levelMap }
    , m_type{ type }
    , m_currentRow{ platformRow }
    , m_startCol{ startCol }
    , m_scaleX{ scaleX }
    , m_scaleY{ scaleY }
    , m_offsetX{ offsetX }
    , m_offsetY{ offsetY }
    , m_player{ player }
    , m_allPieces{ allPieces }
    , m_cups{ cups }
    , m_fallingY{ static_cast<float>(platformY(platformRow)) }
{
    std::string textureName;
    switch (type)
    {
    case BurgerType::TopBun:  textureName = "bt_top_bun.png"; break;
    case BurgerType::Patty:   textureName = "bt_patty.png";   break;
    case BurgerType::Lettuce: textureName = "bt_lettuce.png"; break;
    case BurgerType::BotBun:  textureName = "bt_bot_bun.png"; break;
    case BurgerType::Tomato:  textureName = "bt_tomato.png";  break;
    case BurgerType::Cheese:  textureName = "bt_cheese.png";  break;
    }
    m_texture = dae::ResourceManager::GetInstance().LoadTexture(textureName);
    glm::vec2 size = m_texture->GetSize();
    m_pieceW = size.x;
    m_pieceH = size.y;
    m_segW   = m_pieceW / 4.f;
}

// Gives the X coordinate of the left edge of the piece in world coordinates
float BurgerPieceComponent::GetLeftEdgeX() const
{
    return static_cast<float>(ladderX(m_startCol)) - m_pieceW * 0.5f;
}

int BurgerPieceComponent::FindLandingRow() const
{
    float centerX = GetLeftEdgeX() + m_pieceW * 0.5f;

    // Nearest platform below
    int platRow = -1;
    for (int r = m_currentRow + 1; r < 12; ++r)
    {
        if (m_levelMap->FindPlatform(centerX, static_cast<float>(platformY(r)), 1.f))
        {
            platRow = r;
            break;
        }
    }

    // Nearest cup below at this piece's column
    int cupRow = -1;
    if (m_cups)
    {
        for (const auto& cup : *m_cups)
        {
            if (cup.col == m_startCol && cup.row > m_currentRow)
            {
                if (cupRow < 0 || cup.row < cupRow)
                    cupRow = cup.row;
            }
        }
    }

    // Return whichever is closer (cups can catch pieces before they reach a platform)
    if (platRow < 0) return cupRow;
    if (cupRow  < 0) return platRow;
    return std::min(platRow, cupRow);
}

void BurgerPieceComponent::CheckPlayerPress()
{
    // Should be changed, we are using sprite positions instead of gameobject positions
    float px = m_player->GetSpritePosX();
    float py = m_player->GetSpritePosY();

    if (std::abs(py - static_cast<float>(platformY(m_currentRow))) > 1.f)
        return;

    float x0 = GetLeftEdgeX();
    for (int i = 0; i < 4; ++i)
    {
        if (m_pressed[i]) continue;
        float segX0 = x0 + static_cast<float>(i) * m_segW;
        float segX1 = segX0 + m_segW;
        if (px >= segX0 && px < segX1)
        {
            m_pressed[i] = true;
            m_segmentDrop[i] = MAX_DROP;
        }
    }
}

bool BurgerPieceComponent::AllPressed() const
{
    for (int i = 0; i < 4; ++i)
        if (!m_pressed[i]) return false;
    return true;
}

void BurgerPieceComponent::StartFalling()
{
    m_targetRow = FindLandingRow();
    if (m_targetRow < 0) return;

    // Use cup bottom y if landing in a cup, otherwise the platform row y
    m_targetY = static_cast<float>(platformY(m_targetRow));
    if (m_cups)
    {
        for (const auto& cup : *m_cups)
        {
            if (cup.col == m_startCol && cup.row == m_targetRow)
            {
                m_targetY = CUP_BOTTOM_Y;
                break;
            }
        }
    }

    m_fallingY = static_cast<float>(platformY(m_currentRow));
    m_state = State::Falling;
}

void BurgerPieceComponent::PushDown()
{
    if (m_state != State::Idle) return;
    StartFalling();
}

void BurgerPieceComponent::OnLanded()
{
    m_currentRow = m_targetRow;
    m_fallingY = m_targetY; // already set correctly for both platforms and cups

    for (int i = 0; i < 4; ++i)
    {
        m_pressed[i] = false;
        m_segmentDrop[i] = 0.f;
    }

    // Check if we landed in a cup, stack on top of any pieces already there
    if (m_cups)
    {
        for (const auto& cup : *m_cups)
        {
            if (cup.col == m_startCol && cup.row == m_currentRow)
            {
                int stackCount = 0;
                for (auto* other : *m_allPieces)
                {
                    if (other != this && other->m_state == State::Dead
                        && other->m_startCol == m_startCol)
                        ++stackCount;
                }
                // Stack upward from cup bottom: first piece bottom at CUP_BOTTOM_Y
                m_fallingY = CUP_BOTTOM_Y - static_cast<float>(stackCount) * m_pieceH;
                m_state = State::Dead;
                return;
            }
        }
    }

    // Regular platform landing: push any overlapping pieces below
    float myX0 = GetLeftEdgeX();
    float myX1 = myX0 + m_pieceW;
    for (auto* peer : *m_allPieces)
    {
        if (peer == this || peer->m_state != State::Idle || peer->m_currentRow != m_currentRow)
            continue;

        float pX0 = peer->GetLeftEdgeX();
        float pX1 = pX0 + peer->m_pieceW;
        if (myX0 < pX1 && pX0 < myX1)
            peer->PushDown();
    }

    m_state = State::Idle;
}

void BurgerPieceComponent::Update()
{
    if (m_state == State::Dead) return;

    const float dt = dae::Time::GetInstance().GetDeltaTime();

    if (m_state == State::Idle)
    {
        CheckPlayerPress();
        if (AllPressed())
        {
            StartFalling();
            for (int i = 0; i < 4; ++i)
            {
                m_pressed[i] = false;
                m_segmentDrop[i] = 0.f;
            }
        }
    }
    else // Falling
    {
        m_fallingY += FALL_SPEED * dt;
        if (m_fallingY >= m_targetY)
        {
            m_fallingY = m_targetY;
            OnLanded();
        }
    }
}

void BurgerPieceComponent::Render()
{
    if (!m_texture) return;

    SDL_Renderer* renderer = dae::Renderer::GetInstance().GetSDLRenderer();
    SDL_Texture* tex = m_texture->GetSDLTexture();

    float x0 = GetLeftEdgeX();
    float baseY = m_fallingY;

    for (int i = 0; i < 4; ++i)
    {
        float drop = (m_state == State::Idle) ? m_segmentDrop[i] : 0.f;

        SDL_FRect src{};
        src.x = static_cast<float>(i) * m_segW;
        src.y = 0.f;
        src.w = m_segW;
        src.h = m_pieceH;

        SDL_FRect dst{};
        dst.x = m_offsetX + (x0 + static_cast<float>(i) * m_segW) * m_scaleX;
        dst.y = m_offsetY + (baseY - m_pieceH + drop) * m_scaleY;
        dst.w = m_segW * m_scaleX;
        dst.h = m_pieceH * m_scaleY;

        SDL_RenderTexture(renderer, tex, &src, &dst);
    }
}

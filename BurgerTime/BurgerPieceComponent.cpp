#include "BurgerPieceComponent.h"
#include "PlatformMovementComponent.h"
#include "Renderer.h"
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
                                            std::vector<BurgerPieceComponent*>* allPieces)
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
    , m_fallingY{ static_cast<float>(platformY(platformRow)) }
{
}

// Gives the X coordinate of the left edge of the piece in world coordinates
float BurgerPieceComponent::GetLeftEdgeX() const
{
    return static_cast<float>(ladderX(m_startCol));
}

int BurgerPieceComponent::FindLandingRow() const
{
    float centerX = GetLeftEdgeX() + PIECE_W * 0.5f;
    for (int r = m_currentRow + 1; r < 12; ++r)
    {
        if (m_levelMap->FindPlatform(centerX, static_cast<float>(platformY(r)), 1.f))
            return r;
    }
	// temporary: if no platform found, return -1 to indicate falling off the bottom
    return -1;
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
        float segX0 = x0 + static_cast<float>(i) * SEG_W;
        float segX1 = segX0 + SEG_W;
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
    m_fallingY = static_cast<float>(platformY(m_currentRow));

    for (int i = 0; i < 4; ++i)
    {
        m_pressed[i] = false;
        m_segmentDrop[i] = 0.f;
    }

    float myX0 = GetLeftEdgeX();
    float myX1 = myX0 + PIECE_W;
    for (auto* peer : *m_allPieces)
    {
        if (peer == this || peer->m_state != State::Idle || peer->m_currentRow != m_currentRow)
            continue;

        float pX0 = peer->GetLeftEdgeX();
        float pX1 = pX0 + PIECE_W;
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
        float targetY = static_cast<float>(platformY(m_targetRow));
        if (m_fallingY >= targetY)
        {
            m_fallingY = targetY;
            OnLanded();
        }
    }
}

void BurgerPieceComponent::Render()
{
    if (m_state == State::Dead) return;

    SDL_Color color{};
    switch (m_type)
    {
    case BurgerType::TopBun:  color = { 200, 100,  40, 255 }; break;
    case BurgerType::Patty:   color = { 120,  50,  10, 255 }; break;
    case BurgerType::Lettuce: color = {  60, 160,  50, 255 }; break;
    case BurgerType::BotBun:  color = { 230, 160,  50, 255 }; break;
    }

    float baseY = m_fallingY;

    SDL_Renderer* renderer = dae::Renderer::GetInstance().GetSDLRenderer();
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    float x0 = GetLeftEdgeX();
    for (int i = 0; i < 4; ++i)
    {
        float drop = (m_state == State::Idle) ? m_segmentDrop[i] : 0.f;
        float segSpriteX = x0 + static_cast<float>(i) * SEG_W;
        float segSpriteY = baseY - PIECE_H + drop;

        SDL_FRect rect{};
        rect.x = m_offsetX + segSpriteX * m_scaleX;
        rect.y = m_offsetY + segSpriteY * m_scaleY;
        rect.w = SEG_W * m_scaleX;
        rect.h = PIECE_H * m_scaleY;
        SDL_RenderFillRect(renderer, &rect);
    }
}

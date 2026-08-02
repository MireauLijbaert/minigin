#include "BurgerPieceComponent.h"
#include "PlatformMovementComponent.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "TimeSingleton.h"
#include <SDL3/SDL.h>
#include <cmath>

BurgerPieceComponent::BurgerPieceComponent(dae::GameObject& owner,
                                            const dae::LevelMap* levelMap,
                                            BurgerType type,
                                            int platformRow,
                                            int startCol,
                                            const LevelTransform& transform,
                                            PlatformMovementComponent* player,
                                            std::vector<BurgerPieceComponent*>* allPieces,
                                            const std::vector<CupDef>* cups)
    : BaseComponent(owner)
    , m_levelMap{ levelMap }
    , m_type{ type }
    , m_currentRow{ platformRow }
    , m_startCol{ startCol }
    , m_worldCenterX{ transform.WorldX(static_cast<float>(GridLadderX(startCol))) }
    , m_yTolerance{ 2.f * transform.scaleY }
    , m_maxDrop{ 3.f * transform.scaleY }
    , m_fallSpeed{ 40.f * transform.scaleY }
    , m_cupBottomY{ transform.WorldY(186.f) }
    , m_player{ player }
    , m_allPieces{ allPieces }
    , m_cups{ cups }
    , m_fallingY{ transform.WorldY(static_cast<float>(GridPlatformY(platformRow))) }
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
    m_pieceW = size.x * transform.scaleX;
    m_pieceH = size.y * transform.scaleY;
    m_segW   = m_pieceW / 4.f;
}

float BurgerPieceComponent::GetLeftEdgeX() const
{
    return m_worldCenterX - m_pieceW * 0.5f;
}

void BurgerPieceComponent::CheckPlayerPress()
{
    float px = m_player->GetPosX();
    float py = m_player->GetPosY();

    if (std::abs(py - m_fallingY) > m_yTolerance)
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
            m_segmentDrop[i] = m_maxDrop;
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
    // Find nearest platform below in world coords
    const dae::PlatformRow* plat = m_levelMap->FindNextPlatformBelow(m_worldCenterX, m_fallingY);

    // Find nearest cup below at this column
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

    if (!plat && cupRow < 0) return;

    bool hitCupFirst = plat && cupRow >= 0 ? (cupRow < plat->row) : (cupRow >= 0);

    if (hitCupFirst)
    {
        m_targetRow = cupRow;
        m_targetY = m_cupBottomY;
    }
    else
    {
        m_targetRow = plat->row;
        m_targetY = plat->y;
    }

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
    m_fallingY = m_targetY;

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
                m_fallingY = m_cupBottomY - static_cast<float>(stackCount) * m_pieceH;
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
        m_fallingY += m_fallSpeed * dt;
        if (m_fallingY >= m_targetY)
        {
            m_fallingY = m_targetY;
            OnLanded();
        }
    }
}

void BurgerPieceComponent::Render()
{
	// Render the 4 segments of the burger piece, applying any drop offsets for pressed segments
    if (!m_texture) return;

    SDL_Renderer* renderer = dae::Renderer::GetInstance().GetSDLRenderer();
    SDL_Texture* tex = m_texture->GetSDLTexture();

    float x0 = GetLeftEdgeX();
    float baseY = m_fallingY;

    for (int i = 0; i < 4; ++i)
    {
        float drop = (m_state == State::Idle) ? m_segmentDrop[i] : 0.f;

        SDL_FRect src{};
        src.x = static_cast<float>(i) * (m_texture->GetSize().x / 4.f);
        src.y = 0.f;
        src.w = m_texture->GetSize().x / 4.f;
        src.h = m_texture->GetSize().y;

        SDL_FRect dst{};
        dst.x = x0 + static_cast<float>(i) * m_segW;
        dst.y = baseY - m_pieceH + drop;
        dst.w = m_segW;
        dst.h = m_pieceH;

        SDL_RenderTexture(renderer, tex, &src, &dst);
    }
}

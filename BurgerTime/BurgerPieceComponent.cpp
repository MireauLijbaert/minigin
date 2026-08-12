#include "BurgerPieceComponent.h"
#include "EnemyComponent.h"
#include "PlatformMovementComponent.h"
#include "ScoreManager.h"
#include "ScorePopupManager.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "TimeSingleton.h"
#include "Event.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>

BurgerPieceComponent::BurgerPieceComponent(dae::GameObject& owner,
                                            const dae::LevelMap* levelMap,
                                            BurgerType type,
                                            int platformRow,
                                            int startCol,
                                            const LevelTransform& transform,
                                            PlatformMovementComponent* player,
                                            std::vector<BurgerPieceComponent*>* allPieces,
                                            const std::vector<CupDef>* cups,
                                            std::vector<EnemyComponent*>* enemies)
    : BaseComponent(owner)
    , m_levelMap{ levelMap }
    , m_type{ type }
    , m_currentRow{ platformRow }
    , m_startCol{ startCol }
    , m_worldCenterX{ transform.WorldX(static_cast<float>(GridLadderX(startCol))) }
    , m_yTolerance{ 2.f * transform.scaleY }
    , m_maxDrop{ 3.f * transform.scaleY }
    , m_fallSpeed{ 40.f * transform.scaleY }
    , m_player{ player }
    , m_allPieces{ allPieces }
    , m_cups{ cups }
    , m_enemies{ enemies }
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
    float leftEdge = GetLeftEdgeX();
    for (auto* player : { m_player, m_player2 })
    {
        if (!player || !player->IsAlive()) continue;
        float px = player->GetPosX();
        float py = player->GetPosY();
        if (std::abs(py - m_fallingY) > m_yTolerance) continue;
        for (int i = 0; i < 4; ++i)
        {
            if (m_pressed[i]) continue;
            float segX0 = leftEdge + static_cast<float>(i) * m_segW;
            float segX1 = segX0 + m_segW;
            if (px >= segX0 && px < segX1)
            {
                m_pressed[i] = true;
                m_segmentDrop[i] = m_maxDrop;
                m_subject.NotifyObservers(dae::Event("BurgerSegmentPressed"), GetOwner());
            }
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

    // Find nearest cup below at this column (and capture its actual world Y)
    int   cupRow    = -1;
    float cupWorldY = 0.f;
    if (m_cups)
    {
        for (const auto& cup : *m_cups)
        {
            if (cup.col == m_startCol && cup.row > m_currentRow)
            {
                if (cupRow < 0 || cup.row < cupRow)
                {
                    cupRow    = cup.row;
                    cupWorldY = cup.worldY;
                }
            }
        }
    }

    if (!plat && cupRow < 0) return;

    bool hitCupFirst = plat && cupRow >= 0 ? (cupRow < plat->row) : (cupRow >= 0);

    if (hitCupFirst)
    {
        m_targetRow = cupRow;
        m_targetY   = cupWorldY;   // land at this cup's actual world position
    }
    else
    {
        m_targetRow = plat->row;
        m_targetY = plat->y;
    }

    // Catch any enemies standing on this burger's platform row
    if (m_caughtEnemies.empty() && m_enemies)
    {
        float leftEdge = GetLeftEdgeX();
        for (auto* enemy : *m_enemies)
        {
            if (!enemy->IsAlive()) continue;
            if (std::abs(enemy->GetPosY() - m_fallingY) < m_yTolerance
                && enemy->GetPosX() >= leftEdge && enemy->GetPosX() <= leftEdge + m_pieceW)
            {
                m_caughtEnemies.push_back(enemy);
                enemy->CatchByBurger();
            }
        }

        if (!m_caughtEnemies.empty())
            m_extraFloors = 2;
    }

    m_startFallingY = m_fallingY;
    m_state = State::Falling;
    m_subject.NotifyObservers(dae::Event("BurgerDropped"), GetOwner());
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

    // Squish enemies on the landing platform (skip any carried enemies)
    if (m_enemies)
    {
        float leftEdge = GetLeftEdgeX();
        for (auto* enemy : *m_enemies)
        {
            if (!enemy->IsAlive()) continue;
            bool carried = false;
            for (auto* c : m_caughtEnemies) if (c == enemy) { carried = true; break; }
            if (carried) continue;
            if (std::abs(enemy->GetPosY() - m_fallingY) < m_yTolerance
                && enemy->GetPosX() >= leftEdge && enemy->GetPosX() <= leftEdge + m_pieceW)
            {
                int pts = enemy->GetSquishScore();
                ScoreManager::GetInstance().AddScore(pts);
                ScorePopupManager::GetInstance().Spawn(pts, enemy->GetPosX(), enemy->GetPosY());
                m_subject.NotifyObservers(dae::Event("EnemySquished"), GetOwner());
                enemy->Squish(); // plays squish anim then dies
            }
        }
    }

    // Check cup, carried enemies recover and climb back up, then burger settles.
    if (m_cups)
    {
        for (const auto& cup : *m_cups)
        {
            if (cup.col == m_startCol && cup.row == m_currentRow)
            {
                if (!m_caughtEnemies.empty())
                {
                    static const int fallScores[] = { 500, 1000, 2000, 4000 };
                    int pts = fallScores[std::min((int)m_caughtEnemies.size() - 1, 3)];
                    ScoreManager::GetInstance().AddScore(pts);
                    float sumX = 0.f;
                    for (auto* e : m_caughtEnemies) sumX += e->GetPosX();
                    ScorePopupManager::GetInstance().Spawn(pts,
                        sumX / static_cast<float>(m_caughtEnemies.size()), m_fallingY);
                    for (auto* enemy : m_caughtEnemies) enemy->RecoverFromBurger(m_fallingY);
                    m_caughtEnemies.clear();
                }
                int stackCount = 0;
                for (auto* other : *m_allPieces)
                    if (other != this && other->m_state == State::Dead
                        && other->m_startCol == m_startCol
                        && other->m_currentRow == cup.row)
                        ++stackCount;
                // Stack at the cup's own world Y, not a hardcoded screen bottom
                m_fallingY = cup.worldY - static_cast<float>(stackCount) * m_pieceH;
                m_extraFloors = 0;
                m_state = State::Dead;
                return;
            }
        }
    }

    // Push any overlapping pieces on this platform (every landing, intermediate or final)
    {
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
    }

    // Bounce through extra floors, carried enemies keep riding
    if (m_extraFloors > 0)
    {
        --m_extraFloors;
        StartFalling();
        if (m_state == State::Falling) return;
        // No platform found below, fall through to final landing
    }

    // Final landing: kill carried enemies and award score
    if (!m_caughtEnemies.empty())
    {
        static const int fallScores[] = { 500, 1000, 2000, 4000 };
        int pts = fallScores[std::min((int)m_caughtEnemies.size() - 1, 3)];
        ScoreManager::GetInstance().AddScore(pts);
        float sumX = 0.f;
        for (auto* e : m_caughtEnemies) sumX += e->GetPosX();
        ScorePopupManager::GetInstance().Spawn(pts,
            sumX / static_cast<float>(m_caughtEnemies.size()), m_fallingY);
        m_subject.NotifyObservers(dae::Event("EnemyFell"), GetOwner());
        // Enemies recover at the landing platform, then walk normally.
        for (auto* e : m_caughtEnemies) e->RecoverFromBurger(m_fallingY);
        m_caughtEnemies.clear();
    }

    m_extraFloors = 0;
    m_state = State::Idle;
    m_subject.NotifyObservers(dae::Event("BurgerLanded"), GetOwner());
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

        // Kill any enemy on a ladder that the burger falls through
        if (m_enemies)
        {
            float x0 = GetLeftEdgeX();
            for (auto* enemy : *m_enemies)
            {
                if (!enemy->IsAlive()) continue;
                bool alreadyCaught = false;
                for (auto* c : m_caughtEnemies)
                    if (c == enemy) { alreadyCaught = true; break; }
                if (alreadyCaught) continue;

                if (enemy->GetPosX() >= x0 && enemy->GetPosX() <= x0 + m_pieceW
                    && enemy->GetPosY() >= m_startFallingY
                    && enemy->GetPosY() <= m_fallingY + m_yTolerance
                    && std::abs(enemy->GetPosY() - m_targetY) > m_yTolerance // not on landing platform
                    && !m_levelMap->FindPlatform(enemy->GetPosX(), enemy->GetPosY(), m_yTolerance * 2.f))
                {
                    int pts = enemy->GetSquishScore();
                    ScoreManager::GetInstance().AddScore(pts);
                    ScorePopupManager::GetInstance().Spawn(pts, enemy->GetPosX(), enemy->GetPosY());
                    enemy->Squish(); // same squish anim whether on platform or ladder
                }
            }
        }

        for (auto* enemy : m_caughtEnemies)
            enemy->SetFallingY(m_fallingY);
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

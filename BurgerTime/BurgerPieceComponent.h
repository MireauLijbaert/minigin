#pragma once
#include "BaseComponent.h"
#include "LevelMap.h"
#include <vector>

enum class BurgerType { TopBun, Patty, Lettuce, BotBun };

class PlatformMovementComponent;

class BurgerPieceComponent : public dae::BaseComponent
{
public:
    BurgerPieceComponent(dae::GameObject& owner,
                         const dae::LevelMap* levelMap,
                         BurgerType type,
                         int platformRow,
                         int startCol,
                         float scaleX, float scaleY,
                         float offsetX, float offsetY,
                         PlatformMovementComponent* player,
                         std::vector<BurgerPieceComponent*>* allPieces);

    void Update() override;
    void Render() override;

    void PushDown();

private:
    const dae::LevelMap* m_levelMap;
    BurgerType m_type;
    int m_currentRow;
    int m_startCol;
    float m_scaleX, m_scaleY, m_offsetX, m_offsetY;
    PlatformMovementComponent* m_player;
    std::vector<BurgerPieceComponent*>* m_allPieces;

    bool  m_pressed[4]{};
    float m_segmentDrop[4]{};

    enum class State { Idle, Falling, Dead };
    State m_state{ State::Idle };
    float m_fallingY{};
    int   m_targetRow{ -1 };

    static constexpr float PIECE_W    = 24.f;
    static constexpr float SEG_W      = 6.f;
    static constexpr float PIECE_H    = 4.f;
    static constexpr float MAX_DROP   = 3.f;
    static constexpr float FALL_SPEED = 40.f;

    float GetLeftEdgeX() const;
    int   FindLandingRow() const;
    void  CheckPlayerPress();
    bool  AllPressed() const;
    void  StartFalling();
    void  OnLanded();
};

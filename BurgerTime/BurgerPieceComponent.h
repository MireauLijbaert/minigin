#pragma once
#include "BaseComponent.h"
#include "LevelLoader.h"
#include "LevelMap.h"
#include "Texture2D.h"
#include <memory>
#include <vector>

enum class BurgerType { TopBun, Patty, Lettuce, BotBun, Tomato, Cheese };

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
                         std::vector<BurgerPieceComponent*>* allPieces,
                         const std::vector<CupDef>* cups);

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
    const std::vector<CupDef>* m_cups;

    bool  m_pressed[4]{};
    float m_segmentDrop[4]{};

    enum class State { Idle, Falling, Dead };
    State m_state{ State::Idle };
    float m_fallingY{};
    int   m_targetRow{ -1 };
    float m_targetY{};      // exact sprite-y to fall toward (accounts for cup offset)

    std::shared_ptr<dae::Texture2D> m_texture;
    float m_pieceW{};
    float m_pieceH{};
    float m_segW{};

    static constexpr float MAX_DROP      = 3.f;
    static constexpr float FALL_SPEED   = 40.f;
    static constexpr float CUP_BOTTOM_Y = 186.f; // sprite y where pieces rest in cups

    float GetLeftEdgeX() const;
    int   FindLandingRow() const;
    void  CheckPlayerPress();
    bool  AllPressed() const;
    void  StartFalling();
    void  OnLanded();
};

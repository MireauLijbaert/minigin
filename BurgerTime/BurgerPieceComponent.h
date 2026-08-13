#pragma once
#include "BaseComponent.h"
#include "Subject.h"
#include "LevelLoader.h"
#include "LevelMap.h"
#include "Texture2D.h"
#include <functional>
#include <memory>
#include <vector>

class EnemyComponent;
class VersusEnemyPlayerComponent;

enum class BurgerType { TopBun, Patty, Lettuce, BotBun, Tomato, Cheese };

class PlatformMovementComponent;

class BurgerPieceComponent : public dae::BaseComponent
{
public:
    dae::Subject& GetSubject() { return m_subject; }
    BurgerPieceComponent(dae::GameObject& owner,
                         const dae::LevelMap* levelMap,
                         BurgerType type,
                         int platformRow,
                         int startCol,
                         const LevelTransform& transform,
                         PlatformMovementComponent* player,
                         std::vector<BurgerPieceComponent*>* allPieces,
                         const std::vector<CupDef>* cups,
                         std::vector<EnemyComponent*>* enemies = nullptr);

    void Update() override;
    void Render() override;

    void PushDown();
    bool IsInCup() const { return m_state == State::Dead; }
    void SetPlayer2(PlatformMovementComponent* p2) { m_player2 = p2; }
    void SetVersusEnemy(VersusEnemyPlayerComponent* vep) { m_versusEnemy = vep; }
    // Called once per cup-landing event; wire in Main.cpp to BonusItemComponent::OnBurgerInCup()
    void SetOnCupLanded(std::function<void()> cb) { m_onCupLanded = std::move(cb); }

private:
    const dae::LevelMap* m_levelMap;
    BurgerType m_type;
    int m_currentRow;
    int m_startCol;
    float m_worldCenterX;
    float m_yTolerance;
    float m_maxDrop;
    float m_fallSpeed;
    PlatformMovementComponent* m_player;
    PlatformMovementComponent* m_player2{ nullptr };
    std::vector<BurgerPieceComponent*>* m_allPieces;
    const std::vector<CupDef>* m_cups;
    std::vector<EnemyComponent*>* m_enemies{ nullptr };
    VersusEnemyPlayerComponent*   m_versusEnemy{ nullptr };
    bool                          m_versusEnemyCaught{ false };
    std::function<void()>         m_onCupLanded{};
    std::vector<EnemyComponent*> m_caughtEnemies;

    bool  m_pressed[4]{};
    float m_segmentDrop[4]{};

    enum class State { Idle, Falling, Dead };
    State m_state{ State::Idle };
    float m_fallingY{};
    float m_startFallingY{};
    int   m_targetRow{ -1 };
    float m_targetY{};
    int   m_extraFloors{ 0 }; 

    std::shared_ptr<dae::Texture2D> m_texture;
    float m_pieceW{}; 
    float m_pieceH{};
    float m_segW{};  

    dae::Subject m_subject;
    float GetLeftEdgeX() const;
    void  CheckPlayerPress();
    bool  AllPressed() const;
    void  StartFalling();
    void  OnLanded();
};

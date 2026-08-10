#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#if _DEBUG && __has_include(<vld.h>)
#include <vld.h>
#endif

#include "Minigin.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "Scene.h"
#include "GameObject.h"
#include "RenderComponent.h"
#include "TextComponent.h"
#include "HealthComponent.h"
#include "LevelLoader.h"
#include "PlatformMovementComponent.h"
#include "BurgerPieceComponent.h"
#include "EnemyComponent.h"
#include "PepperComponent.h"
#include "ScoreManager.h"
#include "LevelManagerComponent.h"
#include "DebugLevelComponent.h"
#include "GameOverWatcherComponent.h"
#include "GameOverScreenComponent.h"
#include "BonusItemComponent.h"
#include "BtSounds.h"
#include "ServiceLocator.h"
#include "SoundSystem.h"
#include "SoundObserver.h"
#include "AnimatedSpriteComponent.h"
#include "CharacterAnimators.h"

#include <filesystem>
#include <string>
namespace fs = std::filesystem;

static constexpr float TOP_MARGIN    = 50.f;
static constexpr float BOTTOM_MARGIN = 20.f;

static constexpr float SPRITE_W = 208.f;
static constexpr float SPRITE_H = 187.f;

static constexpr float LEVEL_DST_H    = 576.f - TOP_MARGIN - BOTTOM_MARGIN;
static constexpr float LEVEL_DST_W    = SPRITE_W * (LEVEL_DST_H / SPRITE_H);
static constexpr float LEVEL_OFFSET_X = (1024.f - LEVEL_DST_W) * 0.5f;
static constexpr float LEVEL_OFFSET_Y = TOP_MARGIN;

static constexpr float CHAR_SPRITE_W = 16.f;
static constexpr float CHAR_SPRITE_H = 16.f;

static constexpr int PLAYER_LIVES = 3;
static constexpr int MAX_LEVEL    = 6;

static int s_currentLevel = 1;
static int s_currentLives = PLAYER_LIVES;

static void LoadGameOver();
static void LoadLevel(int levelNum);

// ---- Enemy spawn data -------------------------------------------------------

struct EnemySpawnDef
{
    EnemyType   type;
    float       spawnX;   // sprite-space X (<0 = off left, >208 = off right)
    float       spawnY;   // sprite-space Y (platform row height)
    float       delay;    // seconds before this enemy starts moving
};

// Sprite-space Y for each platform row: GridPlatformY(row) = 1 + row*16
static constexpr float TOP_ROW_Y    =  1.f;   // row 0
static constexpr float BOTTOM_ROW_Y = 145.f;  // row 9
static constexpr float OFF_LEFT_X   = -16.f;
static constexpr float OFF_RIGHT_X  = 224.f;

static const std::vector<EnemySpawnDef> s_levelSpawns[] =
{
    // Level 1: egg bottom-right, hotdog bottom-left, hotdog top-right, hotdog top-left
    {
        { EnemyType::Egg,    OFF_RIGHT_X, BOTTOM_ROW_Y, 0.5f },
        { EnemyType::Hotdog, OFF_LEFT_X,  BOTTOM_ROW_Y, 1.0f },
        { EnemyType::Hotdog, OFF_RIGHT_X, TOP_ROW_Y,    1.5f },
        { EnemyType::Hotdog, OFF_LEFT_X,  TOP_ROW_Y,    2.0f },
    },
    // Levels 2-6: placeholder
    {
        { EnemyType::Hotdog, OFF_RIGHT_X, BOTTOM_ROW_Y, 0.5f },
        { EnemyType::Hotdog, OFF_LEFT_X,  BOTTOM_ROW_Y, 1.0f },
        { EnemyType::Hotdog, OFF_RIGHT_X, TOP_ROW_Y,    1.5f },
        { EnemyType::Hotdog, OFF_LEFT_X,  TOP_ROW_Y,    2.0f },
    },
    {
        { EnemyType::Hotdog, OFF_RIGHT_X, BOTTOM_ROW_Y, 0.5f },
        { EnemyType::Hotdog, OFF_LEFT_X,  BOTTOM_ROW_Y, 1.0f },
        { EnemyType::Egg,    OFF_RIGHT_X, TOP_ROW_Y,    1.5f },
        { EnemyType::Egg,    OFF_LEFT_X,  TOP_ROW_Y,    2.0f },
    },
    {
        { EnemyType::Hotdog, OFF_RIGHT_X, BOTTOM_ROW_Y, 0.5f },
        { EnemyType::Egg,    OFF_LEFT_X,  BOTTOM_ROW_Y, 1.0f },
        { EnemyType::Pickle, OFF_RIGHT_X, TOP_ROW_Y,    1.5f },
        { EnemyType::Hotdog, OFF_LEFT_X,  TOP_ROW_Y,    2.0f },
    },
    {
        { EnemyType::Egg,    OFF_RIGHT_X, BOTTOM_ROW_Y, 0.5f },
        { EnemyType::Egg,    OFF_LEFT_X,  BOTTOM_ROW_Y, 1.0f },
        { EnemyType::Pickle, OFF_RIGHT_X, TOP_ROW_Y,    1.5f },
        { EnemyType::Pickle, OFF_LEFT_X,  TOP_ROW_Y,    2.0f },
    },
    {
        { EnemyType::Pickle, OFF_RIGHT_X, BOTTOM_ROW_Y, 0.5f },
        { EnemyType::Egg,    OFF_LEFT_X,  BOTTOM_ROW_Y, 1.0f },
        { EnemyType::Pickle, OFF_RIGHT_X, TOP_ROW_Y,    1.5f },
        { EnemyType::Egg,    OFF_LEFT_X,  TOP_ROW_Y,    2.0f },
    },
};

static void SetupEnemyClips(AnimatedSpriteComponent* anim, EnemyType type)
{
    switch (type)
    {
    case EnemyType::Egg:
        anim->AddClip("walk_d",  "bt_egg_walk_d.png",   2, 6.f);
        anim->AddClip("walk_h",  "bt_egg_walk_h.png",   2, 6.f);
        anim->AddClip("walk_u",  "bt_egg_walk_u.png",   2, 6.f);
        anim->AddClip("squish",  "bt_egg_squish.png",   4, 6.f, false);
        anim->AddClip("stunned", "bt_egg_stunned.png",  2, 4.f);
        break;
    case EnemyType::Pickle:
        anim->AddClip("walk_d",  "bt_pickle_walk_d.png",   2, 6.f);
        anim->AddClip("walk_h",  "bt_pickle_walk_h.png",   2, 6.f);
        anim->AddClip("walk_u",  "bt_pickle_walk_u.png",   2, 6.f);
        anim->AddClip("squish",  "bt_pickle_squish.png",   4, 6.f, false);
        anim->AddClip("stunned", "bt_pickle_stunned.png",  2, 4.f);
        break;
    default: // Hotdog
        anim->AddClip("walk_d",  "bt_hotdog_walk_d.png",   2, 6.f);
        anim->AddClip("walk_h",  "bt_hotdog_walk_h.png",   2, 6.f);
        anim->AddClip("walk_u",  "bt_hotdog_walk_u.png",   2, 6.f);
        anim->AddClip("squish",  "bt_hotdog_squish.png",   4, 6.f, false);
        anim->AddClip("stunned", "bt_hotdog_stunned.png",  2, 4.f);
        break;
    }
}

static void LoadLevel(int levelNum)
{
    s_currentLevel = levelNum;

    static LevelData levelData;
    static std::vector<BurgerPieceComponent*> burgers;
    static std::vector<EnemyComponent*> enemies;
    burgers.clear();
    enemies.clear();

    const float scaleX = LEVEL_DST_W / SPRITE_W;
    const float scaleY = LEVEL_DST_H / SPRITE_H;
    const LevelTransform transform{ scaleX, scaleY, LEVEL_OFFSET_X, LEVEL_OFFSET_Y };
    const float charW = CHAR_SPRITE_W * scaleX;
    const float charH = CHAR_SPRITE_H * scaleY;

    levelData = LevelLoader::Load("Data/bt_level" + std::to_string(levelNum) + ".txt", transform);

    // Configure popup scale to match character size in screen coords.
    ScorePopupManager::GetInstance().SetDisplaySize(charW, charH);
    ScorePopupManager::GetInstance().Reset();

    auto& scene = dae::SceneManager::GetInstance().CreateScene();
    auto font      = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 20);
    auto smallFont = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 14);

    dae::ServiceLocator::GetSoundSystem().PlayMusicAfter(BtSounds::BGM_LEVEL, BtSounds::SFX_GAME_START, 128);

    // Background
    {
        auto bgObj = std::make_unique<dae::GameObject>();
        bgObj->SetLocalPosition(LEVEL_OFFSET_X, LEVEL_OFFSET_Y);
        auto bgRender = std::make_unique<dae::RenderComponent>(*bgObj);
        bgRender->SetTexture("bt_level" + std::to_string(levelNum) + ".png");
        bgRender->SetSize(LEVEL_DST_W, LEVEL_DST_H);
        bgObj->AddComponent(std::move(bgRender));
        scene.Add(std::move(bgObj));
    }

    // Player
    auto playerObj = std::make_unique<dae::GameObject>();

    auto playerAnim = std::make_unique<AnimatedSpriteComponent>(*playerObj, charW, charH);
    playerAnim->AddClip("idle",     "bt_player_idle.png",     1, 1.f);          // standing still (front-facing)
    playerAnim->AddClip("idle_u",   "bt_player_idle_u.png",   1, 1.f);          // standing still (back-turned, after moving up)
    playerAnim->AddClip("walk_h",   "bt_player_walk_l.png",   3, 8.f);          // sideways, base=LEFT, flip for RIGHT
    playerAnim->AddClip("walk_u",   "bt_player_walk_u.png",   3, 8.f);          // climbing up
    playerAnim->AddClip("walk_d",   "bt_player_walk_d.png",   3, 8.f);          // walking down (front-facing)
    playerAnim->AddClip("die",      "bt_player_die.png",      6, 6.f, false);   // death, non-looping
    playerAnim->AddClip("pepper_d", "bt_player_pepper_d.png", 1, 1.f);          // throw down
    playerAnim->AddClip("pepper_h", "bt_player_pepper_h.png", 1, 1.f);          // throw sideways
    playerAnim->AddClip("pepper_u", "bt_player_pepper_u.png", 1, 1.f);          // throw up
    AnimatedSpriteComponent* playerAnimPtr = playerAnim.get();
    playerObj->AddComponent(std::move(playerAnim));

    auto playerHealth = std::make_unique<dae::HealthComponent>(*playerObj, s_currentLives);
    dae::HealthComponent* playerHealthPtr = playerHealth.get();
    playerObj->AddComponent(std::move(playerHealth));

    auto playerMove = std::make_unique<PlatformMovementComponent>(
        *playerObj, levelData.map.get(), levelData.playerStart, charW, charH, transform
    );
    playerMove->SetHealthComponent(playerHealthPtr);
    playerMove->SetEnemies(&enemies);
    PlatformMovementComponent* playerMovePtr = playerMove.get();
    playerObj->AddComponent(std::move(playerMove));

    auto pepper = std::make_unique<PepperComponent>(
        *playerObj, playerMovePtr, &enemies, charW, charH, 5
    );
    PepperComponent* pepperPtr = pepper.get();
    playerObj->AddComponent(std::move(pepper));

    // Player animator (queries PlatformMovementComponent facing dir each frame)
    auto playerAnimator = std::make_unique<PlayerAnimatorComponent>(
        *playerObj, playerMovePtr, playerAnimPtr);
    PlayerAnimatorComponent* playerAnimatorPtr = playerAnimator.get();
    playerObj->AddComponent(std::move(playerAnimator));

    // Wire pepper throw animator so the throw pose plays for 0.5s
    pepperPtr->SetPepperFiredCallback([playerAnimatorPtr]() {
        playerAnimatorPtr->OnPepperFired();
    });

    scene.Add(std::move(playerObj));

    // Enemies spawn data is per-level (type, off-screen position, stagger delay)
    const auto& spawnDefs = s_levelSpawns[std::min(levelNum - 1, MAX_LEVEL - 1)];
    for (const auto& def : spawnDefs)
    {
        glm::vec2 spawnPos{ transform.WorldX(def.spawnX), transform.WorldY(def.spawnY) };

        auto enemyObj  = std::make_unique<dae::GameObject>();

        // Animated sprite (replaces static RenderComponent)
        auto enemyAnim = std::make_unique<AnimatedSpriteComponent>(*enemyObj, charW, charH);
        SetupEnemyClips(enemyAnim.get(), def.type);
        AnimatedSpriteComponent* enemyAnimPtr = enemyAnim.get();
        enemyObj->AddComponent(std::move(enemyAnim));

        auto enemyComp = std::make_unique<EnemyComponent>(
            *enemyObj, levelData.map.get(), spawnPos, charW, charH, transform, playerMovePtr, def.type
        );
        EnemyComponent* enemyPtr = enemyComp.get();
        enemies.push_back(enemyPtr);
        enemyObj->AddComponent(std::move(enemyComp));

        // Animator drives clip/flip based on enemy state
        enemyObj->AddComponent(std::make_unique<EnemyAnimatorComponent>(
            *enemyObj, enemyPtr, enemyAnimPtr));

        scene.Add(std::move(enemyObj));

        // Stagger initial entry so enemies don't all appear at once
        enemyPtr->Reset(def.delay);
    }

    // Burger pieces
    for (const auto& bdef : levelData.burgers)
    {
        auto burgerObj = std::make_unique<dae::GameObject>();
        auto burgerComp = std::make_unique<BurgerPieceComponent>(
            *burgerObj,
            levelData.map.get(),
            static_cast<BurgerType>(bdef.type),
            bdef.row,
            bdef.startCol,
            transform,
            playerMovePtr,
            &burgers,
            &levelData.cups,
            &enemies
        );
        burgers.push_back(burgerComp.get());
        burgerObj->AddComponent(std::move(burgerComp));
        scene.Add(std::move(burgerObj));
    }

    // Debug overlay (F1 to toggle)
    {
        auto dbg = std::make_unique<dae::GameObject>();
        dbg->AddComponent(std::make_unique<DebugLevelComponent>(
            *dbg, levelData.map.get(), &levelData.cups, transform
        ));
        scene.Add(std::move(dbg));
    }

    // Bonus item, spawns at the center of the level
    BonusItemComponent* bonusPtr = nullptr;
    {
        glm::vec2 bonusPos{
            transform.WorldX(104.f),   // horizontal center of the 208px-wide sprite
            transform.WorldY(96.f)     // roughly mid-level vertically
        };
        auto bonusObj = std::make_unique<dae::GameObject>();
        auto bonusComp = std::make_unique<BonusItemComponent>(
            *bonusObj, bonusPos, charW, playerMovePtr, pepperPtr,
            /*score*/   500,
            /*first*/   10.f,
            /*active*/  10.f,
            /*respawn*/ 25.f
        );
        bonusPtr = bonusComp.get();
        bonusObj->AddComponent(std::move(bonusComp));
        scene.Add(std::move(bonusObj));
    }

    // Level completion (N to skip, auto on all burgers in cups)
    LevelManagerComponent* mgrPtr = nullptr;
    {
        auto mgr = std::make_unique<dae::GameObject>();
        auto mgrComp = std::make_unique<LevelManagerComponent>(
            *mgr, &burgers,
            [playerHealthPtr]()
            {
                s_currentLives = playerHealthPtr->GetLives();
                int next = (s_currentLevel % MAX_LEVEL) + 1;
                dae::SceneManager::GetInstance().RequestLoad([next]()
                {
                    dae::SceneManager::GetInstance().ClearAll();
                    LoadLevel(next);
                });
            }
        );
        mgrPtr = mgrComp.get();
        mgr->AddComponent(std::move(mgrComp));
        scene.Add(std::move(mgr));
    }

    // Game over watcher
    {
        auto watcher = std::make_unique<dae::GameObject>();
        watcher->AddComponent(std::make_unique<GameOverWatcherComponent>(
            *watcher, playerHealthPtr,
            []()
            {
                dae::SceneManager::GetInstance().RequestLoad([]()
                {
                    dae::SceneManager::GetInstance().ClearAll();
                    LoadGameOver();
                });
            }
        ));
        scene.Add(std::move(watcher));
    }

    // Wire SoundObserver to all components that fire audio events
    static SoundObserver soundObserver;
    playerMovePtr->GetSubject().AddObserver(&soundObserver);
    pepperPtr->GetSubject().AddObserver(&soundObserver);
    for (auto* enemy : enemies)
        enemy->GetSubject().AddObserver(&soundObserver);
    for (auto* burger : burgers)
        burger->GetSubject().AddObserver(&soundObserver);
    if (bonusPtr) bonusPtr->GetSubject().AddObserver(&soundObserver);
    if (mgrPtr)   mgrPtr->GetSubject().AddObserver(&soundObserver);

    // HUD layout constants
    // Left panel:  x = 0 .. LEVEL_OFFSET_X (~231px)
    // Right panel: x = LEVEL_OFFSET_X + LEVEL_DST_W .. 1024 (~793..1024px)
    const float leftCX  = LEVEL_OFFSET_X * 0.5f;          // ~115px centre
    const float rightX0 = LEVEL_OFFSET_X + LEVEL_DST_W;   // ~793px
    const float rightCX = rightX0 + (1024.f - rightX0) * 0.5f; // ~909px
    const float iconW   = charW;
    const float iconH   = charH;

    // ---- Top centre: HI-SCORE ----
    {
        auto obj = std::make_unique<dae::GameObject>();
        obj->SetLocalPosition(512.f - 30.f, 6.f);   // centred in 1024px window
        auto render = std::make_unique<dae::RenderComponent>(*obj);
        dae::RenderComponent* rp = render.get();
        obj->AddComponent(std::move(render));
        obj->AddComponent(std::make_unique<dae::TextComponent>(*obj, rp, "HI-SCORE", smallFont));
        scene.Add(std::move(obj));
    }
    {
        auto obj = std::make_unique<dae::GameObject>();
        obj->SetLocalPosition(512.f - 20.f, 22.f);
        auto render = std::make_unique<dae::RenderComponent>(*obj);
        dae::RenderComponent* rp = render.get();
        obj->AddComponent(std::move(render));
        auto text = std::make_unique<dae::TextComponent>(*obj, rp,
            std::to_string(ScoreManager::GetInstance().GetHiScore()), font);
        dae::TextComponent* tp = text.get();
        obj->AddComponent(std::move(text));
        obj->AddComponent(std::make_unique<HiScoreDisplayComponent>(*obj, tp));
        scene.Add(std::move(obj));
    }

    // ---- Left panel ----

    // "1UP" static label
    {
        auto obj = std::make_unique<dae::GameObject>();
        obj->SetLocalPosition(leftCX - 16.f, 6.f);
        auto render = std::make_unique<dae::RenderComponent>(*obj);
        dae::RenderComponent* rp = render.get();
        obj->AddComponent(std::move(render));
        obj->AddComponent(std::make_unique<dae::TextComponent>(*obj, rp, "1UP", smallFont));
        scene.Add(std::move(obj));
    }

    // Score value (updates via observer)
    {
        auto obj = std::make_unique<dae::GameObject>();
        obj->SetLocalPosition(leftCX - 20.f, 22.f);
        auto render = std::make_unique<dae::RenderComponent>(*obj);
        dae::RenderComponent* rp = render.get();
        obj->AddComponent(std::move(render));
        auto text = std::make_unique<dae::TextComponent>(*obj, rp, "0", font);
        dae::TextComponent* tp = text.get();
        obj->AddComponent(std::move(text));
        obj->AddComponent(std::make_unique<ScoreDisplayComponent>(*obj, tp));
        scene.Add(std::move(obj));
    }

    // Life icons (sprite-based, observer)
    {
        auto obj = std::make_unique<dae::GameObject>();
        obj->SetLocalPosition(0.f, 0.f);
        const float livesStartX = 12.f;
        const float livesY      = LEVEL_OFFSET_Y + LEVEL_DST_H - iconH - 8.f;
        obj->AddComponent(std::make_unique<LivesSpriteComponent>(
            *obj, playerHealthPtr, iconW, iconH, livesStartX, livesY));
        scene.Add(std::move(obj));
    }

    // ---- Right panel ----

    // "PEPPER" static label
    {
        auto obj = std::make_unique<dae::GameObject>();
        obj->SetLocalPosition(rightCX - 26.f, 6.f);
        auto render = std::make_unique<dae::RenderComponent>(*obj);
        dae::RenderComponent* rp = render.get();
        obj->AddComponent(std::move(render));
        obj->AddComponent(std::make_unique<dae::TextComponent>(*obj, rp, "PEPPER", smallFont));
        scene.Add(std::move(obj));
    }

    // Pepper count (updates via observer)
    {
        auto obj = std::make_unique<dae::GameObject>();
        obj->SetLocalPosition(rightCX - 6.f, 22.f);
        auto render = std::make_unique<dae::RenderComponent>(*obj);
        dae::RenderComponent* rp = render.get();
        obj->AddComponent(std::move(render));
        auto text = std::make_unique<dae::TextComponent>(*obj, rp, "5", font);
        dae::TextComponent* tp = text.get();
        obj->AddComponent(std::move(text));
        obj->AddComponent(std::make_unique<PepperDisplayComponent>(*obj, tp, pepperPtr));
        scene.Add(std::move(obj));
    }

    // Level number label + value
    {
        auto obj = std::make_unique<dae::GameObject>();
        obj->SetLocalPosition(rightCX - 20.f, 44.f);
        auto render = std::make_unique<dae::RenderComponent>(*obj);
        dae::RenderComponent* rp = render.get();
        obj->AddComponent(std::move(render));
        obj->AddComponent(std::make_unique<dae::TextComponent>(
            *obj, rp, "LV " + std::to_string(s_currentLevel), smallFont));
        scene.Add(std::move(obj));
    }

    // ---- Score popups (update + render each frame) ----
    {
        auto obj = std::make_unique<dae::GameObject>();
        obj->SetLocalPosition(0.f, 0.f);
        obj->AddComponent(std::make_unique<ScorePopupComponent>(*obj));
        scene.Add(std::move(obj));
    }

    // ---- Mute toggle (M key) bottom of right panel ----
    {
        auto obj = std::make_unique<dae::GameObject>();
        const float muteY = LEVEL_OFFSET_Y + LEVEL_DST_H - 20.f;
        obj->SetLocalPosition(rightX0 + 8.f, muteY);
        auto render = std::make_unique<dae::RenderComponent>(*obj);
        dae::RenderComponent* rp = render.get();
        obj->AddComponent(std::move(render));
        auto text = std::make_unique<dae::TextComponent>(*obj, rp, "[M] SOUND", smallFont);
        dae::TextComponent* tp = text.get();
        obj->AddComponent(std::move(text));
        obj->AddComponent(std::make_unique<MuteToggleComponent>(*obj, tp));
        scene.Add(std::move(obj));
    }
}

static void LoadGameOver()
{
    int finalScore  = ScoreManager::GetInstance().GetScore();
    int finalHiScore = ScoreManager::GetInstance().GetHiScore();

    auto& scene = dae::SceneManager::GetInstance().CreateScene();
    auto bigFont   = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 48);
    auto smallFont = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 24);

    // "GAME OVER"
    {
        auto obj = std::make_unique<dae::GameObject>();
        obj->SetLocalPosition(340.f, 200.f);
        auto render = std::make_unique<dae::RenderComponent>(*obj);
        dae::RenderComponent* renderPtr = render.get();
        obj->AddComponent(std::move(render));
        obj->AddComponent(std::make_unique<dae::TextComponent>(*obj, renderPtr, "GAME OVER", bigFont));
        scene.Add(std::move(obj));
    }

    // Score
    {
        auto obj = std::make_unique<dae::GameObject>();
        obj->SetLocalPosition(400.f, 280.f);
        auto render = std::make_unique<dae::RenderComponent>(*obj);
        dae::RenderComponent* renderPtr = render.get();
        obj->AddComponent(std::move(render));
        obj->AddComponent(std::make_unique<dae::TextComponent>(*obj, renderPtr,
            "1UP  " + std::to_string(finalScore), smallFont));
        scene.Add(std::move(obj));
    }

    // Hi-score
    {
        auto obj = std::make_unique<dae::GameObject>();
        obj->SetLocalPosition(400.f, 310.f);
        auto render = std::make_unique<dae::RenderComponent>(*obj);
        dae::RenderComponent* renderPtr = render.get();
        obj->AddComponent(std::move(render));
        obj->AddComponent(std::make_unique<dae::TextComponent>(*obj, renderPtr,
            "BEST " + std::to_string(finalHiScore), smallFont));
        scene.Add(std::move(obj));
    }

    // Prompt
    {
        auto obj = std::make_unique<dae::GameObject>();
        obj->SetLocalPosition(320.f, 360.f);
        auto render = std::make_unique<dae::RenderComponent>(*obj);
        dae::RenderComponent* renderPtr = render.get();
        obj->AddComponent(std::move(render));
        obj->AddComponent(std::make_unique<dae::TextComponent>(*obj, renderPtr,
            "Press Enter or R to play again", smallFont));
        scene.Add(std::move(obj));
    }

    // Mute toggle (no label on game-over screen key still works)
    {
        auto obj = std::make_unique<dae::GameObject>();
        obj->AddComponent(std::make_unique<MuteToggleComponent>(*obj));
        scene.Add(std::move(obj));
    }

    // Restart watcher
    {
        auto obj = std::make_unique<dae::GameObject>();
        obj->AddComponent(std::make_unique<GameOverScreenComponent>(*obj, []()
        {
            ScoreManager::GetInstance().Reset();
            s_currentLives = PLAYER_LIVES;
            dae::SceneManager::GetInstance().RequestLoad([]()
            {
                dae::SceneManager::GetInstance().ClearAll();
                LoadLevel(1);
            });
        }));
        scene.Add(std::move(obj));
    }
}

static void load()
{
    LoadLevel(1);
}

int main(int, char* []) {
    dae::ServiceLocator::RegisterSoundSystem(std::make_unique<dae::SdlSoundSystem>());
#if __EMSCRIPTEN__
    fs::path data_location = "";
#else
    fs::path data_location = "./Data/";
    if (!fs::exists(data_location))
        data_location = "../Data/";
#endif
    dae::Minigin engine(data_location);
    engine.Run(load);
    return 0;
}

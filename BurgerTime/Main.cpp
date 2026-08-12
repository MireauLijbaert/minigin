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
#include "TitleScreenComponent.h"
#include "RoundClearComponent.h"
#include "VersusEnemyPlayerComponent.h"
#include "HighScoreManager.h"
#include "NameEntryComponent.h"

#include <algorithm>
#include <filesystem>
#include <memory>
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

enum class GameMode { SinglePlayer, Coop, Versus };
static GameMode s_gameMode = GameMode::SinglePlayer;

static void LoadNameEntry();
static void LoadTitleScreen();
static void LoadLevel(int levelNum);

// Enemy type helpers (int from LevelLoader → EnemyType enum)
// 0=Hotdog, 1=Egg, 2=Pickle — matches EnemySpawnDef::type

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

    // Levels 7+ use the same map layouts as 1–6 (cycling) but with fixed enemy composition
    const int fileLevel = ((levelNum - 1) % MAX_LEVEL) + 1;
    levelData = LevelLoader::Load("Data/bt_level" + std::to_string(fileLevel) + ".txt", transform);

    // From level 7 onwards enemies are always: K E H K E H (same as level 6)
    if (levelNum > MAX_LEVEL)
    {
        static const int   fixedTypes[]  = { 2, 1, 0, 2, 1, 0 }; // K E H K E H
        static const float fixedDelays[] = { 0.5f, 1.0f, 1.5f, 2.0f, 2.5f, 3.0f };
        int spawnCount = static_cast<int>(levelData.spawnPoints.size());
        levelData.enemies.clear();
        for (int i = 0; i < 6 && spawnCount > 0; ++i)
        {
            const auto& sp = levelData.spawnPoints[i % spawnCount];
            EnemySpawnDef def{};
            def.type   = fixedTypes[i];
            def.spawnX = sp.x;
            def.spawnY = sp.y;
            def.delay  = fixedDelays[i];
            levelData.enemies.push_back(def);
        }
    }

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
        bgRender->SetTexture("bt_level" + std::to_string(fileLevel) + ".png");
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
    playerAnim->AddClip("pepper_d",   "bt_player_pepper_d.png",   1, 1.f);        // throw down
    playerAnim->AddClip("pepper_h",   "bt_player_pepper_h.png",   1, 1.f);        // throw sideways
    playerAnim->AddClip("pepper_u",   "bt_player_pepper_u.png",   1, 1.f);        // throw up
    // 2-frame celebrate: [hands-up | idle], alternates 8 times over ~3.5s at 4.5fps
    playerAnim->AddClip("celebrate",  "bt_player_celebrate.png",   2, 4.5f);
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
    playerMove->SetGamepad(true, 0);   // P1 uses gamepad 0 (D-pad)
    PlatformMovementComponent* playerMovePtr = playerMove.get();
    playerObj->AddComponent(std::move(playerMove));

    auto pepper = std::make_unique<PepperComponent>(
        *playerObj, playerMovePtr, &enemies, charW, charH, 5
    );
    pepper->SetGamepad(true, 0);       // P1 gamepad Y = pepper
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

    // Co-op: Player 2 (WASD + Q for pepper, shares health/lives with player 1)
    PlatformMovementComponent* playerMove2Ptr = nullptr;
    PlayerAnimatorComponent*   playerAnimator2Ptr = nullptr;
    if (s_gameMode == GameMode::Coop)
    {
        auto p2Obj = std::make_unique<dae::GameObject>();

        auto p2Anim = std::make_unique<AnimatedSpriteComponent>(*p2Obj, charW, charH);
        p2Anim->AddClip("idle",      "bt_player_idle.png",       1, 1.f);
        p2Anim->AddClip("idle_u",    "bt_player_idle_u.png",     1, 1.f);
        p2Anim->AddClip("walk_h",    "bt_player_walk_l.png",     3, 8.f);
        p2Anim->AddClip("walk_u",    "bt_player_walk_u.png",     3, 8.f);
        p2Anim->AddClip("walk_d",    "bt_player_walk_d.png",     3, 8.f);
        p2Anim->AddClip("die",       "bt_player_die.png",        6, 6.f, false);
        p2Anim->AddClip("pepper_d",  "bt_player_pepper_d.png",   1, 1.f);
        p2Anim->AddClip("pepper_h",  "bt_player_pepper_h.png",   1, 1.f);
        p2Anim->AddClip("pepper_u",  "bt_player_pepper_u.png",   1, 1.f);
        p2Anim->AddClip("celebrate", "bt_player_celebrate.png",  2, 4.5f);
        // Tint player 2 blue so they're visually distinct
        p2Anim->SetColorMod(100, 180, 255);
        AnimatedSpriteComponent* p2AnimPtr = p2Anim.get();
        p2Obj->AddComponent(std::move(p2Anim));

        // Shared health, player 2 uses player 1's HealthComponent
        auto p2Move = std::make_unique<PlatformMovementComponent>(
            *p2Obj, levelData.map.get(), levelData.playerStart, charW, charH, transform
        );
        p2Move->SetHealthComponent(playerHealthPtr);
        p2Move->SetEnemies(&enemies);
        p2Move->SetKeys({ SDL_SCANCODE_I, SDL_SCANCODE_K, SDL_SCANCODE_J, SDL_SCANCODE_L });
        playerMove2Ptr = p2Move.get();
        p2Obj->AddComponent(std::move(p2Move));

        auto p2Pepper = std::make_unique<PepperComponent>(
            *p2Obj, playerMove2Ptr, &enemies, charW, charH, 5
        );
        p2Pepper->SetFireKey(SDL_SCANCODE_COMMA);
        PepperComponent* p2PepperPtr = p2Pepper.get();
        p2Obj->AddComponent(std::move(p2Pepper));

        auto p2Animator = std::make_unique<PlayerAnimatorComponent>(*p2Obj, playerMove2Ptr, p2AnimPtr);
        PlayerAnimatorComponent* p2AnimatorPtr = p2Animator.get();
        playerAnimator2Ptr = p2AnimatorPtr;
        p2Obj->AddComponent(std::move(p2Animator));

        p2PepperPtr->SetPepperFiredCallback([p2AnimatorPtr]() {
            p2AnimatorPtr->OnPepperFired();
        });

        // Wire sound observer (added later after enemies are set up)
        scene.Add(std::move(p2Obj));
    }

    // Versus: Player 2 controls a Hot Dog (IJKL + comma)
    VersusEnemyPlayerComponent* versusEnemyPtr = nullptr;
    if (s_gameMode == GameMode::Versus)
    {
        // Spawn P2 at the first level spawn point (opposite side from P1)
        glm::vec2 vsSpawn = levelData.playerStart;
        if (!levelData.spawnPoints.empty())
        {
            const auto& sp = levelData.spawnPoints[0];
            vsSpawn = { transform.WorldX(sp.x), transform.WorldY(sp.y) };
        }

        auto vsObj = std::make_unique<dae::GameObject>();

        auto vsAnim = std::make_unique<AnimatedSpriteComponent>(*vsObj, charW, charH);
        vsAnim->AddClip("idle",   "bt_hotdog.png",         1, 1.f);
        vsAnim->AddClip("walk_h", "bt_hotdog_walk_h.png",  2, 8.f);
        vsAnim->AddClip("walk_u", "bt_hotdog_walk_u.png",  2, 8.f);
        vsAnim->AddClip("walk_d", "bt_hotdog_walk_d.png",  2, 8.f);
        AnimatedSpriteComponent* vsAnimPtr = vsAnim.get();
        vsObj->AddComponent(std::move(vsAnim));

        auto vsMove = std::make_unique<PlatformMovementComponent>(
            *vsObj, levelData.map.get(), vsSpawn, charW, charH, transform
        );
        vsMove->SetKeys({ SDL_SCANCODE_I, SDL_SCANCODE_K, SDL_SCANCODE_J, SDL_SCANCODE_L });
        PlatformMovementComponent* vsMovePtr = vsMove.get();
        vsObj->AddComponent(std::move(vsMove));

        auto vsEnemy = std::make_unique<VersusEnemyPlayerComponent>(*vsObj, vsMovePtr, playerMovePtr);
        versusEnemyPtr = vsEnemy.get();
        vsObj->AddComponent(std::move(vsEnemy));

        vsObj->AddComponent(std::make_unique<VersusEnemyAnimatorComponent>(*vsObj, vsMovePtr, vsAnimPtr));

        // P1's pepper can stun the versus hot dog
        pepperPtr->SetVersusTarget(versusEnemyPtr);

        scene.Add(std::move(vsObj));
    }

    // Enemies are defined in the level file under [ENEMIES]
    for (const auto& def : levelData.enemies)
    {
        EnemyType eType = static_cast<EnemyType>(def.type);
        glm::vec2 spawnPos{ transform.WorldX(def.spawnX), transform.WorldY(def.spawnY) };

        auto enemyObj  = std::make_unique<dae::GameObject>();

        // Animated sprite
        auto enemyAnim = std::make_unique<AnimatedSpriteComponent>(*enemyObj, charW, charH);
        SetupEnemyClips(enemyAnim.get(), eType);
        AnimatedSpriteComponent* enemyAnimPtr = enemyAnim.get();
        enemyObj->AddComponent(std::move(enemyAnim));

        auto enemyComp = std::make_unique<EnemyComponent>(
            *enemyObj, levelData.map.get(), spawnPos, charW, charH, transform, playerMovePtr, eType
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

    // Spawn-rotation system: enemies respawn cycling BR,BL,TR,TL,...
    // levelData.spawnPoints is already sorted; convert to world coords.
    {
        auto spawnPts = std::make_shared<std::vector<glm::vec2>>();
        for (const auto& sp : levelData.spawnPoints)
            spawnPts->push_back({ transform.WorldX(sp.x), transform.WorldY(sp.y) });

        // Initial spawns consumed indices 0..enemies.size()-1; next respawn continues at N (wraps)
        auto rotIdx = std::make_shared<int>(static_cast<int>(levelData.enemies.size()));

        for (auto* ep : enemies)
        {
            ep->SetSpawnCallback([spawnPts, rotIdx]() -> glm::vec2
            {
                if (spawnPts->empty()) return { 0.f, 0.f };
                int i = (*rotIdx)++ % static_cast<int>(spawnPts->size());
                return (*spawnPts)[i];
            });
        }
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
        BurgerPieceComponent* burgerPtr = burgerComp.get();
        burgers.push_back(burgerPtr);
        burgerObj->AddComponent(std::move(burgerComp));
        if (playerMove2Ptr) burgerPtr->SetPlayer2(playerMove2Ptr);
        scene.Add(std::move(burgerObj));
    }

    // Co-op: wire player 2 into all enemies
    if (playerMove2Ptr)
    {
        for (auto* ep : enemies)
            ep->SetPlayer2(playerMove2Ptr);
    }

    // F2 = mute/unmute (works in every scene)
    {
        auto muteObj = std::make_unique<dae::GameObject>();
        muteObj->AddComponent(std::make_unique<MuteToggleComponent>(*muteObj));
        scene.Add(std::move(muteObj));
    }

    // Debug overlay (F3 to toggle)
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
        // Cycle through 3 bonus sprites: level 1→ice cream, 2→coffee, 3→fries, loops
        const std::string bonusTex = "bt_bonus_" + std::to_string((fileLevel - 1) % 3 + 1) + ".png";
        auto bonusComp = std::make_unique<BonusItemComponent>(
            *bonusObj, bonusPos, charW, playerMovePtr, pepperPtr,
            /*score*/   500,
            /*first*/   10.f,
            /*active*/  10.f,
            /*respawn*/ 25.f,
            bonusTex
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
                int next = s_currentLevel + 1;
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

    // "ROUND CLEAR!" overlay + win sequence (freeze enemies, celebrate anim)
    {
        auto obj = std::make_unique<dae::GameObject>();
        auto render = std::make_unique<dae::RenderComponent>(*obj);
        dae::RenderComponent* rp = render.get();
        obj->AddComponent(std::move(render));
        auto clearFont = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 40);
        auto text = std::make_unique<dae::TextComponent>(*obj, rp, "", clearFont);
        dae::TextComponent* tp = text.get();
        obj->AddComponent(std::move(text));
        obj->AddComponent(std::make_unique<RoundClearComponent>(
            *obj, mgrPtr, tp, &enemies, playerMovePtr, playerAnimatorPtr, playerMove2Ptr, playerAnimator2Ptr));
        obj->SetLocalPosition(350.f, 250.f);
        scene.Add(std::move(obj));
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
                    LoadNameEntry();
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

static void LoadNameEntry()
{
    const int finalScore = ScoreManager::GetInstance().GetScore();

    auto& scene    = dae::SceneManager::GetInstance().CreateScene();
    auto bigFont   = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 48);
    auto medFont   = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 32);
    auto gridFont  = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 20);

    // "GAME OVER"
    {
        auto obj = std::make_unique<dae::GameObject>();
        obj->SetLocalPosition(340.f, 20.f);
        auto r = std::make_unique<dae::RenderComponent>(*obj);
        dae::RenderComponent* rp = r.get();
        obj->AddComponent(std::move(r));
        obj->AddComponent(std::make_unique<dae::TextComponent>(*obj, rp, "GAME OVER", bigFont));
        scene.Add(std::move(obj));
    }

    // Score line
    {
        auto obj = std::make_unique<dae::GameObject>();
        obj->SetLocalPosition(390.f, 52.f);
        auto r = std::make_unique<dae::RenderComponent>(*obj);
        dae::RenderComponent* rp = r.get();
        obj->AddComponent(std::move(r));
        obj->AddComponent(std::make_unique<dae::TextComponent>(*obj, rp,
            "SCORE  " + std::to_string(finalScore), medFont));
        scene.Add(std::move(obj));
    }

    // Cursor-grid name entry (self-contained: renders name slots + grid + hint)
    {
        auto obj = std::make_unique<dae::GameObject>();
        obj->AddComponent(std::make_unique<NameEntryComponent>(*obj, gridFont, medFont,
            [finalScore](const std::string& name)
            {
                HighScoreManager::GetInstance().AddEntry(name, finalScore);
                HighScoreManager::GetInstance().Save();
                ScoreManager::GetInstance().SetHiScoreFloor(
                    HighScoreManager::GetInstance().GetTopScore());
                ScoreManager::GetInstance().Reset();
                s_currentLives = PLAYER_LIVES;
                dae::SceneManager::GetInstance().RequestLoad([]()
                {
                    dae::SceneManager::GetInstance().ClearAll();
                    LoadTitleScreen();
                });
            }));
        scene.Add(std::move(obj));
    }

    {
        auto obj = std::make_unique<dae::GameObject>();
        obj->AddComponent(std::make_unique<MuteToggleComponent>(*obj));
        scene.Add(std::move(obj));
    }
}

static void LoadTitleScreen()
{
    auto& scene = dae::SceneManager::GetInstance().CreateScene();

    auto titleFont = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 72);
    auto medFont   = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 28);
    auto smallFont = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 20);

    // "BURGER TIME" title
    {
        auto obj = std::make_unique<dae::GameObject>();
        obj->SetLocalPosition(240.f, 150.f);
        auto render = std::make_unique<dae::RenderComponent>(*obj);
        dae::RenderComponent* rp = render.get();
        obj->AddComponent(std::move(render));
        obj->AddComponent(std::make_unique<dae::TextComponent>(*obj, rp, "BURGER TIME", titleFont));
        scene.Add(std::move(obj));
    }

    // "HI-SCORE" label
    {
        auto obj = std::make_unique<dae::GameObject>();
        obj->SetLocalPosition(454.f, 288.f);
        auto render = std::make_unique<dae::RenderComponent>(*obj);
        dae::RenderComponent* rp = render.get();
        obj->AddComponent(std::move(render));
        obj->AddComponent(std::make_unique<dae::TextComponent>(*obj, rp, "HI-SCORE", smallFont));
        scene.Add(std::move(obj));
    }

    // Hi-score value (live-updating)
    {
        auto obj = std::make_unique<dae::GameObject>();
        obj->SetLocalPosition(468.f, 316.f);
        auto render = std::make_unique<dae::RenderComponent>(*obj);
        dae::RenderComponent* rp = render.get();
        obj->AddComponent(std::move(render));
        auto text = std::make_unique<dae::TextComponent>(*obj, rp,
            std::to_string(ScoreManager::GetInstance().GetHiScore()), medFont);
        dae::TextComponent* tp = text.get();
        obj->AddComponent(std::move(text));
        obj->AddComponent(std::make_unique<HiScoreDisplayComponent>(*obj, tp));
        scene.Add(std::move(obj));
    }

    // "PRESS ENTER TO START" blinking prompt
    {
        auto obj = std::make_unique<dae::GameObject>();
        obj->SetLocalPosition(0.f, 0.f);
        auto render = std::make_unique<dae::RenderComponent>(*obj);
        dae::RenderComponent* rp = render.get();
        obj->AddComponent(std::move(render));
        auto text = std::make_unique<dae::TextComponent>(*obj, rp, "PRESS ENTER TO START", smallFont);
        dae::TextComponent* tp = text.get();
        obj->AddComponent(std::move(text));
        auto start1P = []()
        {
            s_gameMode     = GameMode::SinglePlayer;
            s_currentLives = PLAYER_LIVES;
            dae::SceneManager::GetInstance().RequestLoad([]()
            {
                dae::SceneManager::GetInstance().ClearAll();
                LoadLevel(1);
            });
        };
        auto start2P = []()
        {
            s_gameMode     = GameMode::Coop;
            s_currentLives = PLAYER_LIVES;
            dae::SceneManager::GetInstance().RequestLoad([]()
            {
                dae::SceneManager::GetInstance().ClearAll();
                LoadLevel(1);
            });
        };
        auto startVS = []()
        {
            s_gameMode     = GameMode::Versus;
            s_currentLives = PLAYER_LIVES;
            dae::SceneManager::GetInstance().RequestLoad([]()
            {
                dae::SceneManager::GetInstance().ClearAll();
                LoadLevel(1);
            });
        };
        obj->AddComponent(std::make_unique<TitleScreenComponent>(*obj, tp,
            std::move(start1P), std::move(start2P), std::move(startVS)));
        obj->SetLocalPosition(350.f, 430.f);
        scene.Add(std::move(obj));
    }

    // Mute toggle still works on the title screen
    {
        auto obj = std::make_unique<dae::GameObject>();
        obj->AddComponent(std::make_unique<MuteToggleComponent>(*obj));
        scene.Add(std::move(obj));
    }
}

static void load()
{
    LoadTitleScreen();
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
    // Load persisted high scores and prime the in-memory hi-score.
    HighScoreManager::GetInstance().SetFilePath(
        (data_location / "highscores.txt").string());
    HighScoreManager::GetInstance().Load();
    ScoreManager::GetInstance().SetHiScoreFloor(
        HighScoreManager::GetInstance().GetTopScore());

    dae::Minigin engine(data_location);
    engine.Run(load);
    return 0;
}

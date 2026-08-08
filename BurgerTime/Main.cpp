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

    auto& scene = dae::SceneManager::GetInstance().CreateScene();
    auto font = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 20);

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

    auto playerRender = std::make_unique<dae::RenderComponent>(*playerObj);
    playerRender->SetTexture("bt_player.png");
    playerRender->SetSize(charW, charH);
    playerObj->AddComponent(std::move(playerRender));

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
        *playerObj, playerMovePtr, &enemies, charW, charH, 3
    );
    PepperComponent* pepperPtr = pepper.get();
    playerObj->AddComponent(std::move(pepper));

    scene.Add(std::move(playerObj));

    // Enemies
    // Spawn off left/right edges so enemies walk in from outside
    const glm::vec2 enemySpawns[] = {
        { transform.WorldX(-16.f), levelData.playerStart.y },
        { transform.WorldX(224.f), levelData.playerStart.y }
    };
    for (const auto& spawnPos : enemySpawns)
    {
        auto enemyObj    = std::make_unique<dae::GameObject>();
        auto enemyRender = std::make_unique<dae::RenderComponent>(*enemyObj);
        enemyRender->SetTexture("bt_hotdog.png");
        enemyRender->SetSize(charW, charH);
        enemyObj->AddComponent(std::move(enemyRender));

        auto enemyComp = std::make_unique<EnemyComponent>(
            *enemyObj, levelData.map.get(), spawnPos, charW, charH, transform, playerMovePtr
        );
        enemies.push_back(enemyComp.get());
        enemyObj->AddComponent(std::move(enemyComp));
        scene.Add(std::move(enemyObj));
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
    {
        glm::vec2 bonusPos{
            transform.WorldX(104.f),   // horizontal center of the 208px-wide sprite
            transform.WorldY(96.f)     // roughly mid-level vertically
        };
        auto bonusObj = std::make_unique<dae::GameObject>();
        bonusObj->AddComponent(std::make_unique<BonusItemComponent>(
            *bonusObj, bonusPos, charW, playerMovePtr, pepperPtr,
            /*score*/   500,
            /*first*/   10.f,
            /*active*/  10.f,
            /*respawn*/ 25.f
        ));
        scene.Add(std::move(bonusObj));
    }

    // Level completion (N to skip, auto on all burgers in cups)
    {
        auto mgr = std::make_unique<dae::GameObject>();
        mgr->AddComponent(std::make_unique<LevelManagerComponent>(
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
        ));
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

    // HUD: score
    {
        auto obj = std::make_unique<dae::GameObject>();
        obj->SetLocalPosition(10.f, 10.f);
        auto render = std::make_unique<dae::RenderComponent>(*obj);
        dae::RenderComponent* renderPtr = render.get();
        obj->AddComponent(std::move(render));
        auto text = std::make_unique<dae::TextComponent>(*obj, renderPtr, "Score: 0", font);
        dae::TextComponent* textPtr = text.get();
        obj->AddComponent(std::move(text));
        obj->AddComponent(std::make_unique<ScoreDisplayComponent>(*obj, textPtr));
        scene.Add(std::move(obj));
    }

    // HUD: lives
    {
        auto obj = std::make_unique<dae::GameObject>();
        obj->SetLocalPosition(10.f, 32.f);
        auto render = std::make_unique<dae::RenderComponent>(*obj);
        dae::RenderComponent* renderPtr = render.get();
        obj->AddComponent(std::move(render));
        auto text = std::make_unique<dae::TextComponent>(*obj, renderPtr,
            "Lives: " + std::to_string(s_currentLives), font);
        dae::TextComponent* textPtr = text.get();
        obj->AddComponent(std::move(text));
        obj->AddComponent(std::make_unique<LivesDisplayComponent>(*obj, textPtr, playerHealthPtr));
        scene.Add(std::move(obj));
    }

    // HUD: pepper
    {
        auto obj = std::make_unique<dae::GameObject>();
        obj->SetLocalPosition(10.f, 54.f);
        auto render = std::make_unique<dae::RenderComponent>(*obj);
        dae::RenderComponent* renderPtr = render.get();
        obj->AddComponent(std::move(render));
        auto text = std::make_unique<dae::TextComponent>(*obj, renderPtr, "Pepper: 3", font);
        dae::TextComponent* textPtr = text.get();
        obj->AddComponent(std::move(text));
        obj->AddComponent(std::make_unique<PepperDisplayComponent>(*obj, textPtr, pepperPtr));
        scene.Add(std::move(obj));
    }
}

static void LoadGameOver()
{
    int finalScore = ScoreManager::GetInstance().GetScore();

    auto& scene = dae::SceneManager::GetInstance().CreateScene();
    auto bigFont   = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 48);
    auto smallFont = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 24);

    // "GAME OVER"
    {
        auto obj = std::make_unique<dae::GameObject>();
        obj->SetLocalPosition(340.f, 220.f);
        auto render = std::make_unique<dae::RenderComponent>(*obj);
        dae::RenderComponent* renderPtr = render.get();
        obj->AddComponent(std::move(render));
        obj->AddComponent(std::make_unique<dae::TextComponent>(*obj, renderPtr, "GAME OVER", bigFont));
        scene.Add(std::move(obj));
    }

    // Score
    {
        auto obj = std::make_unique<dae::GameObject>();
        obj->SetLocalPosition(400.f, 300.f);
        auto render = std::make_unique<dae::RenderComponent>(*obj);
        dae::RenderComponent* renderPtr = render.get();
        obj->AddComponent(std::move(render));
        obj->AddComponent(std::make_unique<dae::TextComponent>(*obj, renderPtr,
            "Score: " + std::to_string(finalScore), smallFont));
        scene.Add(std::move(obj));
    }

    // Prompt
    {
        auto obj = std::make_unique<dae::GameObject>();
        obj->SetLocalPosition(320.f, 350.f);
        auto render = std::make_unique<dae::RenderComponent>(*obj);
        dae::RenderComponent* renderPtr = render.get();
        obj->AddComponent(std::move(render));
        obj->AddComponent(std::make_unique<dae::TextComponent>(*obj, renderPtr,
            "Press Enter or R to play again", smallFont));
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

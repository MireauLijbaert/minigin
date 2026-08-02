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
#include "LevelLoader.h"
#include "PlatformMovementComponent.h"
#include "BurgerPieceComponent.h"
#include "EnemyComponent.h"

#include <filesystem>
#include <string>
namespace fs = std::filesystem;

static constexpr float TOP_MARGIN    = 50.f;
static constexpr float BOTTOM_MARGIN = 20.f;

static constexpr float SPRITE_W = 208.f;
static constexpr float SPRITE_H = 187.f;

static constexpr float LEVEL_DST_H  = 576.f - TOP_MARGIN - BOTTOM_MARGIN;
static constexpr float LEVEL_DST_W  = SPRITE_W * (LEVEL_DST_H / SPRITE_H);
static constexpr float LEVEL_OFFSET_X = (1024.f - LEVEL_DST_W) * 0.5f;
static constexpr float LEVEL_OFFSET_Y = TOP_MARGIN;

static constexpr float CHAR_SPRITE_W = 16.f;
static constexpr float CHAR_SPRITE_H = 16.f;

static void LoadLevel(int levelNum)
{
    static LevelData levelData;
    static std::vector<BurgerPieceComponent*> burgers;
    burgers.clear();

    const float scaleX = LEVEL_DST_W / SPRITE_W;
    const float scaleY = LEVEL_DST_H / SPRITE_H;
    const LevelTransform transform{ scaleX, scaleY, LEVEL_OFFSET_X, LEVEL_OFFSET_Y };
    const float charW = CHAR_SPRITE_W * scaleX;
    const float charH = CHAR_SPRITE_H * scaleY;

    levelData = LevelLoader::Load("Data/bt_level" + std::to_string(levelNum) + ".txt", transform);

    auto& scene = dae::SceneManager::GetInstance().CreateScene();

    // Background
    auto bgObj = std::make_unique<dae::GameObject>();
    bgObj->SetLocalPosition(LEVEL_OFFSET_X, LEVEL_OFFSET_Y);
    auto bgRender = std::make_unique<dae::RenderComponent>(*bgObj);
    bgRender->SetTexture("bt_level" + std::to_string(levelNum) + ".png");
    bgRender->SetSize(LEVEL_DST_W, LEVEL_DST_H);
    bgObj->AddComponent(std::move(bgRender));
    scene.Add(std::move(bgObj));

    // Player
    auto playerObj = std::make_unique<dae::GameObject>();
    auto playerRender = std::make_unique<dae::RenderComponent>(*playerObj);
    playerRender->SetTexture("bt_player.png");
    playerRender->SetSize(charW, charH);
    playerObj->AddComponent(std::move(playerRender));

    auto playerMove = std::make_unique<PlatformMovementComponent>(
        *playerObj, levelData.map.get(), levelData.playerStart, charW, charH, transform
    );
    PlatformMovementComponent* playerMovePtr = playerMove.get();
    playerObj->AddComponent(std::move(playerMove));
    scene.Add(std::move(playerObj));

    // Enemies at top-left and top-right (ladderX(0)=8, ladderX(8)=200, platformY(0)=1 in sprite coords)
    const glm::vec2 enemySpawns[] = {
        { transform.WorldX(8.f),   transform.WorldY(1.f) },
        { transform.WorldX(200.f), transform.WorldY(1.f) }
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
            &levelData.cups
        );
        burgers.push_back(burgerComp.get());
        burgerObj->AddComponent(std::move(burgerComp));
        scene.Add(std::move(burgerObj));
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

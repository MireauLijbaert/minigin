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

#include <filesystem>
#include <string>
namespace fs = std::filesystem;

static constexpr float TOP_MARGIN    = 50.f;
static constexpr float BOTTOM_MARGIN = 20.f;

// Native sprite size of the level PNGs (after cropping)
static constexpr float SPRITE_W = 208.f;
static constexpr float SPRITE_H = 187.f;

static constexpr float LEVEL_DST_H = 576.f - TOP_MARGIN - BOTTOM_MARGIN;
static constexpr float LEVEL_SCALE  = LEVEL_DST_H / SPRITE_H;
static constexpr float LEVEL_DST_W  = SPRITE_W * LEVEL_SCALE;

static constexpr float LEVEL_OFFSET_X = (1024.f - LEVEL_DST_W) * 0.5f;
static constexpr float LEVEL_OFFSET_Y = TOP_MARGIN;

// Character sprite native size
static constexpr float CHAR_SPRITE_W = 16.f;
static constexpr float CHAR_SPRITE_H = 16.f;

static void LoadLevel(int levelNum)
{
    static LevelData levelData;
    static std::vector<BurgerPieceComponent*> burgers;
    burgers.clear();
    levelData = LevelLoader::Load("Data/bt_level" + std::to_string(levelNum) + ".txt");

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
    float scaleX = LEVEL_DST_W / SPRITE_W;
    float scaleY = LEVEL_DST_H / SPRITE_H;
    float charDisplayW = CHAR_SPRITE_W * scaleX;
    float charDisplayH = CHAR_SPRITE_H * scaleY;

    auto playerObj = std::make_unique<dae::GameObject>();
    auto playerRender = std::make_unique<dae::RenderComponent>(*playerObj);
    playerRender->SetTexture("bt_player.png");
    playerRender->SetSize(charDisplayW, charDisplayH);
    playerObj->AddComponent(std::move(playerRender));

    auto playerMove = std::make_unique<PlatformMovementComponent>(
        *playerObj,
        levelData.map.get(),
        levelData.playerStartSprite,
        scaleX, scaleY,
        LEVEL_OFFSET_X, LEVEL_OFFSET_Y,
        CHAR_SPRITE_W, CHAR_SPRITE_H
    );
    PlatformMovementComponent* playerMovePtr = playerMove.get();
    playerObj->AddComponent(std::move(playerMove));
    scene.Add(std::move(playerObj));

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
            scaleX, scaleY,
            LEVEL_OFFSET_X, LEVEL_OFFSET_Y,
            playerMovePtr,
            &burgers
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

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#if _DEBUG && __has_include(<vld.h>)
#include <vld.h>
#endif

#include "Minigin.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "Scene.h"
#include "InputManager.h"
#include "RenderComponent.h"
#include "Renderer.h"
#include "Commands.h"
#include "GridMovementComponent.h"
#include "SnoBeeComponent.h"
#include "HealthComponent.h"
#include "GameManager.h"
#include "LevelLoader.h"
#include "TextComponent.h"

#include <cmath>
#include <filesystem>
namespace fs = std::filesystem;

static void load()
{
    auto& scene = dae::SceneManager::GetInstance().CreateScene();

    dae::Renderer::GetInstance().SetScale(2.f);

    const int tileSize = 16;
    const float moveSpeed = 128.f;

    // Window logical size at 2x scale: 512 x 288
    // Background image: 223 x 256
    const float bgX = std::floor((512.f - 223.f) / 2.f);
    const float bgY = std::floor((288.f - 256.f) / 2.f);

    // ---------- Background ----------
    auto bg = std::make_unique<dae::GameObject>();
    auto bgRender = std::make_unique<dae::RenderComponent>(*bg);
    bgRender->SetTexture("FullBackgroundPengo.png");
    bg->AddComponent(std::move(bgRender));
    bg->SetLocalPosition(bgX, bgY);
    scene.Add(std::move(bg));

    // ---------- Grid root (all grid objects are children so world = gridRoot.pos + local) ----------
    auto gridRoot = std::make_unique<dae::GameObject>();
    gridRoot->SetLocalPosition(bgX + 8.f, bgY + 8.f);
    dae::GameObject* gridRootPtr = gridRoot.get();
    scene.Add(std::move(gridRoot));

    static LevelData levelData = LevelLoader::Load("Data/level1.txt", scene, tileSize, gridRootPtr);

    // ---------- Player ----------
    auto player = std::make_unique<dae::GameObject>();
    auto playerRender = std::make_unique<dae::RenderComponent>(*player);
    auto playerRenderPtr = playerRender.get();
    player->AddComponent(std::move(playerRender));
    playerRenderPtr->SetTexture("RedPengo.png");
    player->AddComponent(std::make_unique<dae::GridMovementComponent>(
        *player, tileSize, levelData.playerStartCell, levelData.gridSize, moveSpeed, levelData.registry.get(), false
    ));
    player->AddComponent(std::make_unique<dae::HealthComponent>(*player, 3));
    dae::GameObject* playerPtr = player.get();
    player->SetParent(gridRootPtr, false);
    scene.Add(std::move(player));

    // ---------- Input ----------
    auto& input = dae::InputManager::GetInstance();

    input.BindKeyboardInput(SDL_SCANCODE_W, std::make_unique<dae::GridMoveCommand>(*playerPtr, glm::ivec2{ 0, -1 }), dae::InputState::Held);
    input.BindKeyboardInput(SDL_SCANCODE_S, std::make_unique<dae::GridMoveCommand>(*playerPtr, glm::ivec2{ 0,  1 }), dae::InputState::Held);
    input.BindKeyboardInput(SDL_SCANCODE_A, std::make_unique<dae::GridMoveCommand>(*playerPtr, glm::ivec2{-1,  0 }), dae::InputState::Held);
    input.BindKeyboardInput(SDL_SCANCODE_D, std::make_unique<dae::GridMoveCommand>(*playerPtr, glm::ivec2{ 1,  0 }), dae::InputState::Held);
    input.BindKeyboardInput(SDL_SCANCODE_SPACE, std::make_unique<dae::PushCommand>(*playerPtr), dae::InputState::Down);

    // ---------- Victory text ----------
    auto font = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 18);
    auto victoryObj = std::make_unique<dae::GameObject>();
    auto victoryRender = std::make_unique<dae::RenderComponent>(*victoryObj);
    auto* victoryRenderPtr = victoryRender.get();
    victoryObj->AddComponent(std::move(victoryRender));
    auto victoryText = std::make_unique<dae::TextComponent>(*victoryObj, victoryRenderPtr, "", font);
    auto* victoryTextPtr = victoryText.get();
    victoryObj->AddComponent(std::move(victoryText));
    victoryObj->SetLocalPosition(100.f, 120.f); // rough center of playfield
    scene.Add(std::move(victoryObj));

    // ---------- Game Manager ----------
    auto gameManagerObj = std::make_unique<dae::GameObject>();
    auto gameManagerComp = std::make_unique<dae::GameManager>(*gameManagerObj, victoryTextPtr);
    dae::GameManager* gameManager = gameManagerComp.get();
    gameManagerObj->AddComponent(std::move(gameManagerComp));
    scene.Add(std::move(gameManagerObj));

    gameManager->SetPlayer(playerPtr, levelData.playerStartCell);

    // ---------- Sno-bees ----------
    // Helper: create one Sno-bee at a given grid cell
    auto addSnoBee = [&](glm::ivec2 startCell)
    {
        auto snobee = std::make_unique<dae::GameObject>();

        auto render = std::make_unique<dae::RenderComponent>(*snobee);
        render->SetTexture("SnoBee.png");
        snobee->AddComponent(std::move(render));

        snobee->AddComponent(std::make_unique<dae::GridMovementComponent>(
            *snobee, tileSize, startCell, levelData.gridSize, moveSpeed * 0.6f, levelData.registry.get(), false
        ));
        auto snobeeBehavior = std::make_unique<dae::SnoBeeComponent>(
            *snobee, levelData.gridSize, levelData.registry.get(), playerPtr, gameManager
        );
        auto* snobeeBehaviorPtr = snobeeBehavior.get();
        snobee->AddComponent(std::move(snobeeBehavior));

        gameManager->AddSnoBee(snobeeBehaviorPtr, startCell);

        snobee->SetLocalPosition(float(startCell.x * tileSize), float(startCell.y * tileSize));
        snobee->SetParent(gridRootPtr, false);
        scene.Add(std::move(snobee));
    };

    addSnoBee({ 1,  1 });
    addSnoBee({ 11, 1 });
    addSnoBee({ 1, 13 });
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

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
#include "LevelLoader.h"

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
        *player, tileSize, levelData.playerStartCell, levelData.gridSize, moveSpeed, levelData.registry.get()
    ));
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

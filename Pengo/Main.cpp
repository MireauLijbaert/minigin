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
#include "Commands.h"
#include "GridMovementComponent.h"

#include <filesystem>
namespace fs = std::filesystem;

static void load()
{
    auto& scene = dae::SceneManager::GetInstance().CreateScene();

    // ---------- Player ----------
    auto player = std::make_unique<dae::GameObject>();
    auto playerRender = std::make_unique<dae::RenderComponent>(*player);
    auto playerRenderPtr = playerRender.get();
    player->AddComponent(std::move(playerRender));
    playerRenderPtr->SetTexture("RedPengo.png");

    const int tileSize = 32;
    const glm::ivec2 gridSize{ 13, 13 };
    const glm::ivec2 startCell{ 6, 6 };
    const float moveSpeed = 128.f; // pixels per second

    player->AddComponent(std::make_unique<dae::GridMovementComponent>(*player, tileSize, startCell, gridSize, moveSpeed));
    dae::GameObject* playerPtr = player.get();
    scene.Add(std::move(player));

    // ---------- Input ----------
    auto& input = dae::InputManager::GetInstance();

    input.BindKeyboardInput(SDL_SCANCODE_W, std::make_unique<dae::GridMoveCommand>(*playerPtr, glm::ivec2{ 0, -1 }), dae::InputState::Held);
    input.BindKeyboardInput(SDL_SCANCODE_S, std::make_unique<dae::GridMoveCommand>(*playerPtr, glm::ivec2{ 0,  1 }), dae::InputState::Held);
    input.BindKeyboardInput(SDL_SCANCODE_A, std::make_unique<dae::GridMoveCommand>(*playerPtr, glm::ivec2{-1,  0 }), dae::InputState::Held);
    input.BindKeyboardInput(SDL_SCANCODE_D, std::make_unique<dae::GridMoveCommand>(*playerPtr, glm::ivec2{ 1,  0 }), dae::InputState::Held);

    /*
    // ---------- Background ----------
    auto background = std::make_unique<dae::GameObject>();
    ...

    // ---------- FPS Counter ----------
    ...

    // ---------- Player 1 Health/Score Display ----------
    ...

    // ---------- Player 2 ----------
    ...
    */
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

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

#include <filesystem>
namespace fs = std::filesystem;

static void load()
{
    auto& scene = dae::SceneManager::GetInstance().CreateScene();

    dae::Renderer::GetInstance().SetScale(2.f);

    const int tileSize = 16;
    const float moveSpeed = 128.f;

    static LevelData levelData = LevelLoader::Load("Data/level1.txt", scene, tileSize);

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
    scene.Add(std::move(player));

    // ---------- Input ----------
    auto& input = dae::InputManager::GetInstance();

    input.BindKeyboardInput(SDL_SCANCODE_W, std::make_unique<dae::GridMoveCommand>(*playerPtr, glm::ivec2{ 0, -1 }), dae::InputState::Held);
    input.BindKeyboardInput(SDL_SCANCODE_S, std::make_unique<dae::GridMoveCommand>(*playerPtr, glm::ivec2{ 0,  1 }), dae::InputState::Held);
    input.BindKeyboardInput(SDL_SCANCODE_A, std::make_unique<dae::GridMoveCommand>(*playerPtr, glm::ivec2{-1,  0 }), dae::InputState::Held);
    input.BindKeyboardInput(SDL_SCANCODE_D, std::make_unique<dae::GridMoveCommand>(*playerPtr, glm::ivec2{ 1,  0 }), dae::InputState::Held);
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

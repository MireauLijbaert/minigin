#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#if _DEBUG && __has_include(<vld.h>)
#include <vld.h>
#endif

#include "Minigin.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "Scene.h"
#include "RenderComponent.h"
#include "Renderer.h"
#include "GridMovementComponent.h"
#include "SnoBeeComponent.h"
#include "HealthComponent.h"
#include "GameManager.h"
#include "LevelLoader.h"
#include "TextComponent.h"
#include "PengoControllerComponent.h"
#include "ServiceLocator.h"
#include "PengoSounds.h"
#include "InputManager.h"
#include "Command.h"

#include <cmath>
#include <filesystem>
namespace fs = std::filesystem;

static const char* LevelFile(int levelNum)
{
    switch (levelNum)
    {
    case 2:  return "Data/level2.txt";
    case 3:  return "Data/level3.txt";
    default: return "Data/level1.txt";
    }
}

static void LoadLevel(int levelNum)
{
    auto& scene = dae::SceneManager::GetInstance().CreateScene();

    dae::Renderer::GetInstance().SetScale(2.f);

    const int tileSize = 16;
    const float moveSpeed = 85.f;

    const float bgX = std::floor((512.f - 223.f) / 2.f);
    const float bgY = std::floor((288.f - 256.f) / 2.f);

    // ---------- Background ----------
    auto bg = std::make_unique<dae::GameObject>();
    auto bgRender = std::make_unique<dae::RenderComponent>(*bg);
    bgRender->SetTexture("FullBackgroundPengo.png");
    bg->AddComponent(std::move(bgRender));
    bg->SetLocalPosition(bgX, bgY);
    scene.Add(std::move(bg));

    // ---------- Grid root ----------
    auto gridRoot = std::make_unique<dae::GameObject>();
    gridRoot->SetLocalPosition(bgX + 8.f, bgY + 8.f);
    dae::GameObject* gridRootPtr = gridRoot.get();
    scene.Add(std::move(gridRoot));

    // Static so the registry outlives LoadLevel's stack frame
    static LevelData levelData;
    levelData = LevelLoader::Load(LevelFile(levelNum), scene, tileSize, gridRootPtr);

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
    auto pengoComp = std::make_unique<dae::PengoControllerComponent>(*player, moveSpeed, 0, dae::KeyboardScheme::WASD);
    dae::PengoControllerComponent* pengoCompPtr = pengoComp.get();
    player->AddComponent(std::move(pengoComp));
    dae::GameObject* playerPtr = player.get();
    player->SetParent(gridRootPtr, false);
    scene.Add(std::move(player));

    // ---------- Victory text ----------
    auto font = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 18);
    auto victoryObj = std::make_unique<dae::GameObject>();
    auto victoryRender = std::make_unique<dae::RenderComponent>(*victoryObj);
    auto* victoryRenderPtr = victoryRender.get();
    victoryObj->AddComponent(std::move(victoryRender));
    auto victoryText = std::make_unique<dae::TextComponent>(*victoryObj, victoryRenderPtr, "", font);
    auto* victoryTextPtr = victoryText.get();
    victoryObj->AddComponent(std::move(victoryText));
    victoryObj->SetLocalPosition(100.f, 120.f);
    scene.Add(std::move(victoryObj));

    // ---------- Game Manager ----------
    auto gameManagerObj = std::make_unique<dae::GameObject>();
    auto gameManagerComp = std::make_unique<dae::GameManager>(*gameManagerObj, victoryTextPtr);
    dae::GameManager* gameManager = gameManagerComp.get();
    gameManagerObj->AddComponent(std::move(gameManagerComp));
    scene.Add(std::move(gameManagerObj));

    gameManager->SetPlayer(pengoCompPtr, levelData.playerStartCell);
    pengoCompPtr->SetGameManager(gameManager);

    // ---------- Sno-bees ----------
    // Extract from static levelData so lambda captures by value (static locals can't be captured)
    glm::ivec2 gridSize = levelData.gridSize;
    dae::GridRegistry* registry = levelData.registry.get();

    auto addSnoBee = [&scene, tileSize, moveSpeed, playerPtr, gameManager, gridRootPtr, gridSize, registry](glm::ivec2 startCell)
    {
        auto snobee = std::make_unique<dae::GameObject>();

        auto render = std::make_unique<dae::RenderComponent>(*snobee);
        render->SetTexture("SnoBee.png");
        snobee->AddComponent(std::move(render));

        snobee->AddComponent(std::make_unique<dae::GridMovementComponent>(
            *snobee, tileSize, startCell, gridSize, moveSpeed * 0.65f, registry, false
        ));
        auto snobeeBehavior = std::make_unique<dae::SnoBeeComponent>(
            *snobee, gridSize, registry, playerPtr, gameManager
        );
        auto* snobeeBehaviorPtr = snobeeBehavior.get();
        snobee->AddComponent(std::move(snobeeBehavior));

        gameManager->AddSnoBee(snobeeBehaviorPtr, startCell);

        snobee->SetLocalPosition(float(startCell.x * tileSize), float(startCell.y * tileSize));
        snobee->SetParent(gridRootPtr, false);
        scene.Add(std::move(snobee));
    };

    if (!levelData.snoBeeSpawnCells.empty())
    {
        for (const auto& cell : levelData.snoBeeSpawnCells)
            addSnoBee(cell);
    }
    else
    {
        addSnoBee({ 1,  1 });
        addSnoBee({ 11, 1 });
        addSnoBee({ 1, 13 });
    }

    // Wire up egg hatching
    gameManager->SetRegistry(levelData.registry.get());
    for (const auto& cell : levelData.eggCells)
        gameManager->AddEggCell(cell);
    gameManager->SetSnoBeeSpawnFn(addSnoBee);

    // Level progression: cycle 1->2->3->1
    int nextLevel = (levelNum % 3) + 1;
    gameManager->SetOnLevelComplete([nextLevel]()
    {
        dae::SceneManager::GetInstance().RequestLoad([nextLevel]() { LoadLevel(nextLevel); });
    });

    // Start background music
    dae::ServiceLocator::GetSoundSystem().PlayMusic(PengoSounds::BGM, -1);
    dae::ServiceLocator::GetSoundSystem().SetMusicVolume(PengoSounds::MUSIC_VOLUME);

    // F1: skip to next level
    dae::InputManager::GetInstance().BindKeyboardInput(
        SDL_SCANCODE_F1,
        std::make_unique<dae::LambdaCommand>([nextLevel]()
        {
            dae::SceneManager::GetInstance().RequestLoad([nextLevel]() { LoadLevel(nextLevel); });
        }),
        dae::InputState::Down
    );

    // F2: mute / unmute toggle
    static bool s_muted = false;
    dae::InputManager::GetInstance().BindKeyboardInput(
        SDL_SCANCODE_F2,
        std::make_unique<dae::LambdaCommand>([]()
        {
            s_muted = !s_muted;
            dae::ServiceLocator::GetSoundSystem().SetMusicVolume(
                s_muted ? 0 : PengoSounds::MUSIC_VOLUME
            );
        }),
        dae::InputState::Down
    );
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

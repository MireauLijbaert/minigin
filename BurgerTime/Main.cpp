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

#include <filesystem>
#include <string>
namespace fs = std::filesystem;

static void LoadLevel(int levelNum)
{
    auto& scene = dae::SceneManager::GetInstance().CreateScene();

    auto bgObj = std::make_unique<dae::GameObject>();
    auto renderComp = std::make_unique<dae::RenderComponent>(*bgObj);
    renderComp->SetTexture("bt_level" + std::to_string(levelNum) + ".png");
    bgObj->AddComponent(std::move(renderComp));
    scene.Add(std::move(bgObj));
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

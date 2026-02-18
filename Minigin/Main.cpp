#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#if _DEBUG && __has_include(<vld.h>)
#include <vld.h>
#endif

#include "Minigin.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "TextObject.h"
#include "Scene.h"

#include "FPSComponent.h"
#include "TextComponent.h"
#include "RenderComponent.h"

#include "Renderer.h"


#include <filesystem>
namespace fs = std::filesystem;

static void load()
{
	// Base minigin code

	auto& scene = dae::SceneManager::GetInstance().CreateScene();

	auto go = std::make_unique<dae::GameObject>();
	go->SetTexture("background.png");
	scene.Add(std::move(go));

	go = std::make_unique<dae::GameObject>();
	go->SetTexture("logo.png");
	go->SetPosition(358, 180);
	scene.Add(std::move(go));

	auto font = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 36);
	auto to = std::make_unique<dae::TextObject>("Programming 4 Assignment", font);
	to->SetColor({ 255, 255, 0, 255 });
	to->SetPosition(292, 20);
	scene.Add(std::move(to));

	// Make FPS counter
	auto fpsObject = std::make_unique<dae::GameObject>();
	font = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 25);
	auto textComponent = std::make_unique<dae::TextComponent>(*fpsObject, "fps", font);
	fpsObject->AddComponent(std::move(textComponent));
	auto fpsComponent = std::make_unique<dae::FpsComponent>(*fpsObject, fpsObject->GetComponent<dae::TextComponent>());
	fpsObject->SetPosition(20, 50);
	fpsObject->AddComponent(std::move(fpsComponent));
	scene.Add(std::move(fpsObject));
}

int main(int, char*[]) {
#if __EMSCRIPTEN__
	fs::path data_location = "";
#else
	fs::path data_location = "./Data/";
	if(!fs::exists(data_location))
		data_location = "../Data/";
#endif
	dae::Minigin engine(data_location);
	engine.Run(load);
    return 0;
}

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#if _DEBUG && __has_include(<vld.h>)
#include <vld.h>
#endif

#include "Minigin.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "Scene.h"

#include "FPSComponent.h"
#include "TextComponent.h"
#include "RenderComponent.h"
// temporary for testing
#include "RotationComponent.h"
#include "BenchmarkComponent.h"

#include "Renderer.h"

#include <filesystem>
namespace fs = std::filesystem;

static void load()
{
	// Base minigin code

	auto& scene = dae::SceneManager::GetInstance().CreateScene();

	// Will refactor this a bit to not use the extra pointers in between if not needed

	// ---------- Background ----------
	auto background = std::make_unique<dae::GameObject>();
	auto backgroundRender = std::make_unique<dae::RenderComponent>(*background);
	auto backgroundRenderPtr = backgroundRender.get(); // keep raw pointer
	background->AddComponent(std::move(backgroundRender));
	backgroundRenderPtr->SetTexture("background.png");
	background->SetLocalPosition(0, 0);
	scene.Add(std::move(background));

	// ---------- Logo ----------
	auto logo = std::make_unique<dae::GameObject>();
	auto logoRender = std::make_unique<dae::RenderComponent>(*logo);
	auto logoRenderPtr = logoRender.get(); // raw pointer
	logo->AddComponent(std::move(logoRender));
	logoRenderPtr->SetTexture("logo.png");
	logo->SetLocalPosition(358, 180);
	scene.Add(std::move(logo));

	// ---------- Title ----------
	auto textObject = std::make_unique<dae::GameObject>();
	auto renderComponent = std::make_unique<dae::RenderComponent>(*textObject);
	auto renderPtr = renderComponent.get(); // keep raw pointer
	textObject->AddComponent(std::move(renderComponent));
	auto font = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 36);
	auto textComponent = std::make_unique<dae::TextComponent>(
		*textObject, renderPtr, "Programming 4 Assignment", font
	);
	textComponent->SetColor({ 255, 255, 0, 255 });
	textObject->AddComponent(std::move(textComponent));
	textObject->SetLocalPosition(292, 20);
	scene.Add(std::move(textObject));

	// ---------- FPS Counter ----------
	auto fpsObject = std::make_unique<dae::GameObject>();
	renderComponent = std::make_unique<dae::RenderComponent>(*fpsObject);
	renderPtr = renderComponent.get(); // raw pointer
	fpsObject->AddComponent(std::move(renderComponent));
	font = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 25);
	textComponent = std::make_unique<dae::TextComponent>(
		*fpsObject, renderPtr, "fps", font
	);
	fpsObject->AddComponent(std::move(textComponent));
	auto fpsTextPtr = fpsObject->GetComponent<dae::TextComponent>();
	auto fpsComponent = std::make_unique<dae::FpsComponent>(*fpsObject, fpsTextPtr);
	fpsObject->AddComponent(std::move(fpsComponent));
	fpsObject->SetLocalPosition(20, 50);
	scene.Add(std::move(fpsObject));

	// ---------- Benchmark Object ----------
	auto benchmarkObject = std::make_unique<dae::GameObject>();
	auto benchmarkComponent = std::make_unique<dae::BenchmarkComponent>(*benchmarkObject);
	benchmarkObject->AddComponent(std::move(benchmarkComponent));
	scene.Add(std::move(benchmarkObject));

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

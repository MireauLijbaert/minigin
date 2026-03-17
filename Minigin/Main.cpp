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

#include "FPSComponent.h"
#include "TextComponent.h"
#include "RenderComponent.h"
#include "MovementComponent.h"
#include "Commands.h"

#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include "Xinput.h"
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

    // ---------- Player 1 ----------
    auto player1 = std::make_unique<dae::GameObject>();
    auto player1Render = std::make_unique<dae::RenderComponent>(*player1);
    auto player1RenderPtr = player1Render.get();
    player1->AddComponent(std::move(player1Render));
    player1RenderPtr->SetTexture("RedPengo.png");
    player1->SetLocalPosition(200.f, 300.f);

    // Keep raw pointer before moving into scene so commands can reference the object
    dae::GameObject* player1Ptr = player1.get();

    scene.Add(std::move(player1));


    // ---------- Player 2 ----------
    auto player2 = std::make_unique<dae::GameObject>();
    auto player2Render = std::make_unique<dae::RenderComponent>(*player2);
    auto player2RenderPtr = player2Render.get();
    player2->AddComponent(std::move(player2Render));
    player2RenderPtr->SetTexture("PinkPengo.png");
    player2->SetLocalPosition(500.f, 300.f);

    dae::GameObject* player2Ptr = player2.get();

    scene.Add(std::move(player2));


    // ---------- Input Bindings ----------
    auto& input = dae::InputManager::GetInstance();

    //
    // Player 1 - Keyboard WASD
    //
	float speed1 = 100.f; // Player 1 moves at normal speed
    input.BindKeyboardInput(
        SDL_SCANCODE_W,
        std::make_unique<dae::MovementCommand>(*player1Ptr, glm::vec3{ 0.f, -1.f, 0.f }, speed1),
        dae::InputState::Held
    );
    input.BindKeyboardInput(
        SDL_SCANCODE_A,
        std::make_unique<dae::MovementCommand>(*player1Ptr, glm::vec3{ -1.f, 0.f, 0.f }, speed1),
        dae::InputState::Held
    );
    input.BindKeyboardInput(
        SDL_SCANCODE_S,
        std::make_unique<dae::MovementCommand>(*player1Ptr, glm::vec3{ 0.f, 1.f, 0.f }, speed1),
        dae::InputState::Held
    );
    input.BindKeyboardInput(
        SDL_SCANCODE_D,
        std::make_unique<dae::MovementCommand>(*player1Ptr, glm::vec3{ 1.f, 0.f, 0.f }, speed1),
        dae::InputState::Held
    );

    //
    // Player 1 - Gamepad 2 (index 1), D-pad
    //
	
    input.BindGamepadInput(
        XINPUT_GAMEPAD_DPAD_UP,
        std::make_unique<dae::MovementCommand>(*player1Ptr, glm::vec3{ 0.f, -1.f, 0.f }, speed1),
        dae::InputState::Held,
        1
    );
    input.BindGamepadInput(
        XINPUT_GAMEPAD_DPAD_LEFT,
        std::make_unique<dae::MovementCommand>(*player1Ptr, glm::vec3{ -1.f, 0.f, 0.f }, speed1),
        dae::InputState::Held,
        1
    );
    input.BindGamepadInput(
        XINPUT_GAMEPAD_DPAD_DOWN,
        std::make_unique<dae::MovementCommand>(*player1Ptr, glm::vec3{ 0.f, 1.f, 0.f }, speed1),
        dae::InputState::Held,
        1
    );
    input.BindGamepadInput(
        XINPUT_GAMEPAD_DPAD_RIGHT,
        std::make_unique<dae::MovementCommand>(*player1Ptr, glm::vec3{ 1.f, 0.f, 0.f }, speed1),
        dae::InputState::Held,
        1
    );

    //
    // Player 2 - Keyboard IJKL (for testing)
    //
    float speed2 = 200.f; // Player 2 moves at double speed
    input.BindKeyboardInput(
        SDL_SCANCODE_I,
        std::make_unique<dae::MovementCommand>(*player2Ptr, glm::vec3{ 0.f, -1.f, 0.f }, speed2),
        dae::InputState::Held
    );
    input.BindKeyboardInput(
        SDL_SCANCODE_J,
        std::make_unique<dae::MovementCommand>(*player2Ptr, glm::vec3{ -1.f, 0.f, 0.f }, speed2),
        dae::InputState::Held
    );
    input.BindKeyboardInput(
        SDL_SCANCODE_K,
        std::make_unique<dae::MovementCommand>(*player2Ptr, glm::vec3{ 0.f, 1.f, 0.f }, speed2),
        dae::InputState::Held
    );
    input.BindKeyboardInput(
        SDL_SCANCODE_L,
        std::make_unique<dae::MovementCommand>(*player2Ptr, glm::vec3{ 1.f, 0.f, 0.f }, speed2),
        dae::InputState::Held
    );

    //
    // Player 2 - Gamepad 1 (index 0), D-pad
    //
    input.BindGamepadInput(
        XINPUT_GAMEPAD_DPAD_UP,
        std::make_unique<dae::MovementCommand>(*player2Ptr, glm::vec3{ 0.f, -1.f, 0.f }, speed2),
        dae::InputState::Held,
        0
    );
    input.BindGamepadInput(
        XINPUT_GAMEPAD_DPAD_LEFT,
        std::make_unique<dae::MovementCommand>(*player2Ptr, glm::vec3{ -1.f, 0.f, 0.f }, speed2),
        dae::InputState::Held,
        0
    );
    input.BindGamepadInput(
        XINPUT_GAMEPAD_DPAD_DOWN,
        std::make_unique<dae::MovementCommand>(*player2Ptr, glm::vec3{ 0.f, 1.f, 0.f }, speed2),
        dae::InputState::Held,
        0
    );  
    input.BindGamepadInput(
        XINPUT_GAMEPAD_DPAD_RIGHT,
        std::make_unique<dae::MovementCommand>(*player2Ptr, glm::vec3{ 1.f, 0.f, 0.f }, speed2),
        dae::InputState::Held,
        0
    );

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

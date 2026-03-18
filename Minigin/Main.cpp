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
#include "Commands.h"

#include "HealthComponent.h"
#include "HealthDisplayComponent.h"
#include "ScoreComponent.h"
#include "ScoreDisplayComponent.h"

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
    auto player1Health = std::make_unique<dae::HealthComponent>(*player1, 3);
    auto player1HealthPtr = player1Health.get();
    player1->AddComponent(std::move(player1Health));
    auto player1Score = std::make_unique<dae::ScoreComponent>(*player1);
    auto player1ScorePtr = player1Score.get();
    player1->AddComponent(std::move(player1Score));
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
    auto player2Health = std::make_unique<dae::HealthComponent>(*player2, 3);
    auto player2HealthPtr = player2Health.get();
    player2->AddComponent(std::move(player2Health));
    auto player2Score = std::make_unique<dae::ScoreComponent>(*player2);
    auto player2ScorePtr = player2Score.get();
    player2->AddComponent(std::move(player2Score));
    player2RenderPtr->SetTexture("PinkPengo.png");
    player2->SetLocalPosition(500.f, 300.f);

    dae::GameObject* player2Ptr = player2.get();

    scene.Add(std::move(player2));

    // ---------- Player 1 Lives Display ----------
    auto player1LivesObject = std::make_unique<dae::GameObject>();

    auto player1LivesRender = std::make_unique<dae::RenderComponent>(*player1LivesObject);
    auto player1LivesRenderPtr = player1LivesRender.get();
    player1LivesObject->AddComponent(std::move(player1LivesRender));

    auto livesFont = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 20);

    auto player1LivesText = std::make_unique<dae::TextComponent>(
        *player1LivesObject,
        player1LivesRenderPtr,
        "Lives: 3",
        livesFont
    );
    auto player1LivesTextPtr = player1LivesText.get();
    player1LivesObject->AddComponent(std::move(player1LivesText));

    auto player1LivesDisplay = std::make_unique<dae::HealthDisplayComponent>(
        *player1LivesObject,
        &player1HealthPtr->GetSubject(),
        player1LivesTextPtr
    );
    player1LivesObject->AddComponent(std::move(player1LivesDisplay));

    player1LivesObject->SetLocalPosition(20.f, 90.f);
    scene.Add(std::move(player1LivesObject));

    // ---------- Player 1 Score Display ----------
    auto player1ScoreObject = std::make_unique<dae::GameObject>();

    auto player1ScoreRender = std::make_unique<dae::RenderComponent>(*player1ScoreObject);
    auto player1ScoreRenderPtr = player1ScoreRender.get();
    player1ScoreObject->AddComponent(std::move(player1ScoreRender));

    auto player1ScoreText = std::make_unique<dae::TextComponent>(
        *player1ScoreObject,
        player1ScoreRenderPtr,
        "Score: " + std::to_string(player1ScorePtr->GetScore()),
        livesFont
    );
    auto player1ScoreTextPtr = player1ScoreText.get();
    player1ScoreObject->AddComponent(std::move(player1ScoreText));

    auto player1ScoreDisplay = std::make_unique<dae::ScoreDisplayComponent>(
        *player1ScoreObject,
        &player1ScorePtr->GetSubject(),
        player1ScoreTextPtr
    );
    player1ScoreObject->AddComponent(std::move(player1ScoreDisplay));

    player1ScoreObject->SetLocalPosition(20.f, 150.f);
    scene.Add(std::move(player1ScoreObject));

    // ---------- Player 2 Lives Display ----------
    auto player2LivesObject = std::make_unique<dae::GameObject>();

    auto player2LivesRender = std::make_unique<dae::RenderComponent>(*player2LivesObject);
    auto player2LivesRenderPtr = player2LivesRender.get();
    player2LivesObject->AddComponent(std::move(player2LivesRender));

    auto player2LivesText = std::make_unique<dae::TextComponent>(
        *player2LivesObject,
        player2LivesRenderPtr,
        "Lives: 3",
        livesFont
    );
    auto player2LivesTextPtr = player2LivesText.get();
    player2LivesObject->AddComponent(std::move(player2LivesText));

    auto player2LivesDisplay = std::make_unique<dae::HealthDisplayComponent>(
        *player2LivesObject,
        &player2HealthPtr->GetSubject(),
        player2LivesTextPtr
    );
    player2LivesObject->AddComponent(std::move(player2LivesDisplay));

    player2LivesObject->SetLocalPosition(20.f, 120.f);
    scene.Add(std::move(player2LivesObject));

    // ---------- Player 2 Score Display ----------
    auto player2ScoreObject = std::make_unique<dae::GameObject>();

    auto player2ScoreRender = std::make_unique<dae::RenderComponent>(*player2ScoreObject);
    auto player2ScoreRenderPtr = player2ScoreRender.get();
    player2ScoreObject->AddComponent(std::move(player2ScoreRender));

    auto player2ScoreText = std::make_unique<dae::TextComponent>(
        *player2ScoreObject,
        player2ScoreRenderPtr,
        "Score: " + std::to_string(player2ScorePtr->GetScore()),
        livesFont
    );
    auto player2ScoreTextPtr = player2ScoreText.get();
    player2ScoreObject->AddComponent(std::move(player2ScoreText));

    auto player2ScoreDisplay = std::make_unique<dae::ScoreDisplayComponent>(
        *player2ScoreObject,
        &player2ScorePtr->GetSubject(),
        player2ScoreTextPtr
    );
    player2ScoreObject->AddComponent(std::move(player2ScoreDisplay));

    player2ScoreObject->SetLocalPosition(20.f, 180.f);
    scene.Add(std::move(player2ScoreObject));

    // ---------- Controls Text ----------
    auto controlsObject = std::make_unique<dae::GameObject>();

    auto controlsRender = std::make_unique<dae::RenderComponent>(*controlsObject);
    auto controlsRenderPtr = controlsRender.get();
    controlsObject->AddComponent(std::move(controlsRender));

    auto controlsFont = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 16);

    auto controlsText = std::make_unique<dae::TextComponent>(
        *controlsObject,
        controlsRenderPtr,
        "P1 Move: WASD / Gamepad 2 DPad | P1 Damage: E | P1 Score: R |"
        "P2 Move: IJKL / Gamepad 1 DPad | P2 Damage: O | P2 Score: P",
        controlsFont
    );
    controlsObject->AddComponent(std::move(controlsText));

    controlsObject->SetLocalPosition(20.f, 550.f);
    scene.Add(std::move(controlsObject));

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
        dae::GamepadButton::DPadUp,
        std::make_unique<dae::MovementCommand>(*player1Ptr, glm::vec3{ 0.f, -1.f, 0.f }, speed1),
        dae::InputState::Held,
        1
    );
    input.BindGamepadInput(
        dae::GamepadButton::DPadLeft,
        std::make_unique<dae::MovementCommand>(*player1Ptr, glm::vec3{ -1.f, 0.f, 0.f }, speed1),
        dae::InputState::Held,
        1
    );
    input.BindGamepadInput(
        dae::GamepadButton::DPadDown,
        std::make_unique<dae::MovementCommand>(*player1Ptr, glm::vec3{ 0.f, 1.f, 0.f }, speed1),
        dae::InputState::Held,
        1
    );
    input.BindGamepadInput(
        dae::GamepadButton::DPadRight,
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
        dae::GamepadButton::DPadUp,
        std::make_unique<dae::MovementCommand>(*player2Ptr, glm::vec3{ 0.f, -1.f, 0.f }, speed2),
        dae::InputState::Held,
        0
    );
    input.BindGamepadInput(
        dae::GamepadButton::DPadLeft,
        std::make_unique<dae::MovementCommand>(*player2Ptr, glm::vec3{ -1.f, 0.f, 0.f }, speed2),
        dae::InputState::Held,
        0
    );
    input.BindGamepadInput(
        dae::GamepadButton::DPadDown,
        std::make_unique<dae::MovementCommand>(*player2Ptr, glm::vec3{ 0.f, 1.f, 0.f }, speed2),
        dae::InputState::Held,
        0
    );  
    input.BindGamepadInput(
        dae::GamepadButton::DPadRight,
        std::make_unique<dae::MovementCommand>(*player2Ptr, glm::vec3{ 1.f, 0.f, 0.f }, speed2),
        dae::InputState::Held,
        0
    );

    // ---------- Damage Test Bindings ----------

// Player 1 takes damage with E
    input.BindKeyboardInput(
        SDL_SCANCODE_E,
        std::make_unique<dae::TakeDamageCommand>(*player1Ptr, 1),
        dae::InputState::Down
    );

    // Player 2 takes damage with O
    input.BindKeyboardInput(
        SDL_SCANCODE_O,
        std::make_unique<dae::TakeDamageCommand>(*player2Ptr, 1),
        dae::InputState::Down
    );

    // ---------- Score Test Bindings ----------

    // Player 1 gains score with R
    input.BindKeyboardInput(
        SDL_SCANCODE_R,
        std::make_unique<dae::IncreaseScoreCommand>(*player1Ptr, 100),
        dae::InputState::Down
    );

    // Player 2 gains score with P
    input.BindKeyboardInput(
        SDL_SCANCODE_P,
        std::make_unique<dae::IncreaseScoreCommand>(*player2Ptr, 100),
        dae::InputState::Down
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

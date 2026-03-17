// Done in minigin too but things don't work here if it's not in here directly
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include "Xinput.h"

#include <SDL3/SDL.h>
#include <backends/imgui_impl_sdl3.h>

#include "InputManager.h"

#include <unordered_map>


class dae::InputManager::impl
{
public:
	bool ProcessInput() 
	{
		// Keyboard DOWN and UP events
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_EVENT_QUIT) {
				return false;
			}
			if (e.type == SDL_EVENT_KEY_DOWN) {

				for (const auto& binding : m_keyboardCommandsDown)
				{
					if (e.key.scancode == binding.first)
					{
						binding.second->Execute();
					}
				}
			}
			if (e.type == SDL_EVENT_KEY_UP) {

				for (const auto& binding : m_keyboardCommandsUp)
				{
					if (e.key.scancode == binding.first)
					{
						binding.second->Execute();
					}
				}
			}

			//process event for IMGUI
			ImGui_ImplSDL3_ProcessEvent(&e);
		}

		// Keyboard HELD events (uses a different method)
		const bool* keyboardState = SDL_GetKeyboardState(nullptr);

		for (const auto& binding : m_keyboardCommandsHeld)
		{
			if (keyboardState[binding.first])
			{
				binding.second->Execute();
			}
		}

		// Gamepad
		DWORD dwResult;
		// loop through all possible controller slots (0 to XUSER_MAX_COUNT - 1) XUSER_MAX_COUNT is defined in Xinput.h as 4, meaning we can support up to 4 controllers (though could change this to 2 for exam project)
		for (DWORD i = 0; i < XUSER_MAX_COUNT; i++)
		{
			XINPUT_STATE state{};
			dwResult = XInputGetState(i, &state);
			if (dwResult == ERROR_SUCCESS)
			{
				WORD buttonState = state.Gamepad.wButtons;
				WORD previousButtonState = m_previousButtons[i];

				// HELD
				for (const auto& binding : m_gamepadCommandsHeld[i])
				{
					if (buttonState & binding.first)
					{
						binding.second->Execute();
					}
				}

				// DOWN

				for (const auto& binding : m_gamepadCommandsDown[i])
				{
					bool isPressed = (buttonState & binding.first) != 0; // !=0 checks if the button is currently pressed (if bit is 0 then it's not pressed)
					bool wasPressed = (previousButtonState & binding.first) != 0;

					if (!wasPressed && isPressed) // if it is pressed but wasn't before
					{
						binding.second->Execute();
					}
				}

				// UP
				for (const auto& binding : m_gamepadCommandsUp[i])
				{
					bool isPressed = (buttonState & binding.first) != 0; // !=0 checks if the button is currently pressed (if bit is 0 then it's not pressed)
					bool wasPressed = (previousButtonState & binding.first) != 0;

					if (wasPressed && !isPressed) // if it was pressed but isn't now
					{
						binding.second->Execute();
					}
				}

				// Update the previous button state for the next frame
				m_previousButtons[i] = buttonState;
			}
			else
			{
				// Controller is not connected, set it's state to 0 for if it was a previously connected
				m_previousButtons[i] = 0;
			}
		}

		return true;
	}

	void BindKeyboardInput(SDL_Scancode key, std::unique_ptr<Command> command, InputState inputState)
	{
		if (inputState == InputState::Up)
		{
			m_keyboardCommandsUp[key] = std::move(command);
		}
		else if (inputState == InputState::Down)
		{
			m_keyboardCommandsDown[key] = std::move(command);
		}
		else if (inputState == InputState::Held)
		{
			m_keyboardCommandsHeld[key] = std::move(command);
		}
	}

	// Use WORD here, the uint16_t automatically conversts to WORD, no casting needed
	void BindGamepadInput(WORD button, std::unique_ptr<Command> command, InputState inputState, uint32_t gamepadIndex)
	{
		if (gamepadIndex >= XUSER_MAX_COUNT)
		{
			// Invalid gamepad index
			return;
		}
		if (inputState == InputState::Up)
		{
			m_gamepadCommandsUp[gamepadIndex][button] = std::move(command);
		}
		else if (inputState == InputState::Down)
		{
			m_gamepadCommandsDown[gamepadIndex][button] = std::move(command);
		}
		else if (inputState == InputState::Held)
		{
			m_gamepadCommandsHeld[gamepadIndex][button] = std::move(command);
		}
	}


private:
	// 3 different states for keyboard (maybe change this so a vector
	std::unordered_map<SDL_Scancode, std::unique_ptr<Command>> m_keyboardCommandsUp{};
	std::unordered_map<SDL_Scancode, std::unique_ptr<Command>> m_keyboardCommandsDown{};
	std::unordered_map<SDL_Scancode, std::unique_ptr<Command>> m_keyboardCommandsHeld{};
	
	// 3 different states for gamepad, in an array for maximum USER_MAX_COUNT (4 right now) gamepads
	std::unordered_map<WORD, std::unique_ptr<Command>> m_gamepadCommandsUp[XUSER_MAX_COUNT]{};
	std::unordered_map<WORD, std::unique_ptr<Command>> m_gamepadCommandsDown[XUSER_MAX_COUNT]{};
	std::unordered_map<WORD, std::unique_ptr<Command>> m_gamepadCommandsHeld[XUSER_MAX_COUNT]{};

	// Remember the previous button states to be able to determine if a button was just pressed, just released or is being held down
	WORD m_previousButtons[XUSER_MAX_COUNT]{};

};

dae::InputManager::InputManager()
	: pImpl(std::make_unique<impl>())
{
}
dae::InputManager::~InputManager() = default;

bool dae::InputManager::ProcessInput()
{
	return pImpl->ProcessInput();
}

void dae::InputManager::BindKeyboardInput(SDL_Scancode key, std::unique_ptr<Command> command, InputState inputState)
{
	pImpl->BindKeyboardInput(key, std::move(command), inputState);
}

void dae::InputManager::BindGamepadInput(GamepadButton button, std::unique_ptr<Command> command, InputState inputState, uint32_t gamepadIndex)
{
	pImpl->BindGamepadInput(static_cast<WORD>(button), std::move(command), inputState, gamepadIndex);
}

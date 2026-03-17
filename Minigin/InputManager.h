#pragma once
#include "Singleton.h"
#include "memory"
#include "Commands.h"

#include <cstdint>
#include <SDL3/SDL.h>

namespace dae
{
	enum class InputState
	{
		Up,
		Down,
		Held
	};

	enum class GamepadButton : uint16_t
	{
		DPadUp = 0x0001,
		DPadDown = 0x0002,
		DPadLeft = 0x0004,
		DPadRight = 0x0008,
		Start = 0x0010,
		Back = 0x0020,
		LeftThumb = 0x0040,
		RightThumb = 0x0080,
		LeftShoulder = 0x0100,
		RightShoulder = 0x0200,
		A = 0x1000,
		B = 0x2000,
		X = 0x4000,
		Y = 0x8000
	};

	class InputManager final : public Singleton<InputManager>
	{
	public:
		InputManager();
		~InputManager();

		bool ProcessInput();

		void BindKeyboardInput(SDL_Scancode key, std::unique_ptr<Command> command, InputState inputState );
		// Use uint16_t instead of WORD to avoid including windows.h in this header, will convert it to WORD in the cpp file when needed
		void BindGamepadInput(GamepadButton button, std::unique_ptr<Command> command, InputState inputState, uint32_t gamepadIndex);

	private:
		class impl;
		std::unique_ptr<impl> pImpl;
	};

}

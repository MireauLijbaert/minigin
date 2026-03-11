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

	class InputManager final : public Singleton<InputManager>
	{
	public:
		InputManager();
		~InputManager();

		bool ProcessInput();

		void BindKeyboardInput(SDL_Scancode key, std::unique_ptr<Command> command, InputState inputState );
		// Use uint16_t instead of WORD to avoid including windows.h in this header, will convert it to WORD in the cpp file when needed
		void BindGamepadInput(uint16_t button, std::unique_ptr<Command> command, InputState inputState);

	private:
		class impl;
		std::unique_ptr<impl> pImpl;
	};

}

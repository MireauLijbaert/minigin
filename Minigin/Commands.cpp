#pragma once

namespace dae
{
	class Command
	{
	public:
		virtual ~Command() = default;
		virtual void Execute() = 0;
	};

	class MovementCommand : public Command
	{
	public: 
		void Execute() override
		{
			// Implement movement logic here
		}
	};

}

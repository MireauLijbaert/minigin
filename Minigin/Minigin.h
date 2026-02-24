#pragma once
#include <string>
#include <functional>
#include <filesystem>
#include <chrono> 

namespace dae
{
	class Minigin final
	{
	private:
		std::chrono::steady_clock::time_point m_lastTime; // Needed to make last time a member variable instead of a local variable in Run, so that it can be used in RunOneFrame as well
		const int m_ms_per_frame{ 16 }; // Became member variable to make RunOneFrame work
		
		bool m_quit{};
	public:
		explicit Minigin(const std::filesystem::path& dataPath);
		~Minigin();
		void Run(const std::function<void()>& load);
		void RunOneFrame();

		Minigin(const Minigin& other) = delete;
		Minigin(Minigin&& other) = delete;
		Minigin& operator=(const Minigin& other) = delete;
		Minigin& operator=(Minigin&& other) = delete;
	};
}
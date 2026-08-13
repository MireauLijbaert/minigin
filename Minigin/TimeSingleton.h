#pragma once
#include "Singleton.h"

namespace dae

{
	class Time final : public Singleton<Time>
	{
	public:
		float GetDeltaTime() const { return m_paused ? 0.f : m_DeltaTime; }
		void  SetDeltaTime(float deltaTime) { m_DeltaTime = deltaTime; }
		void  SetPaused(bool paused) { m_paused = paused; }
		bool  IsPaused() const { return m_paused; }
	private:
		float m_DeltaTime{};
		bool  m_paused{ false };
		friend class Singleton<Time>;
	};
}
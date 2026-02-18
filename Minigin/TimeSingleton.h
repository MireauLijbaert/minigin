#pragma once
#include "Singleton.h"

namespace dae

{
	class Time final : public Singleton<Time>
	{
	public:
		float GetDeltaTime() const { return m_DeltaTime; }
		void SetDeltaTime(float deltaTime) { m_DeltaTime = deltaTime; }
	private:
		float m_DeltaTime{};
		friend class Singleton<Time>;
	};
}
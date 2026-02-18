#include "FpsComponent.h"
#include "TimeSingleton.h"

void dae::FpsComponent::Update()
{
	float deltaTime{ dae::Time::GetInstance().GetDeltaTime() };
	float fps{ 1 / deltaTime };
	std::string fpsText{ std::to_string(fps) + " FPS" };
	m_DelayUpdate += deltaTime;
	if (m_TextComponent && m_DelayUpdate > 0.2)
	{
		m_DelayUpdate = 0;
		m_TextComponent->SetText(std::to_string(static_cast<int>(fps)) + " FPS");
		m_TextComponent->Update();
	}
}

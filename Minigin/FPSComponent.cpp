#include "FpsComponent.h"
#include "TimeSingleton.h"

void dae::FpsComponent::Update()
{
	float deltaTime{ dae::Time::GetInstance().GetDeltaTime() };
	float fps{ 1 / deltaTime };
	std::string fpsText{ std::to_string(fps) + " FPS" };
	m_delayUpdate += deltaTime;
	if (m_textComponent && m_delayUpdate > 0.2)
	{
		m_delayUpdate = 0;
		m_textComponent->SetText(std::to_string(static_cast<int>(fps)) + " FPS");
	}
}

void dae::FpsComponent::Render()
{
	// Has to be here for build, but doesn't need to do anything as the text component will handle rendering the text, will figure out how to abstract this away later
}
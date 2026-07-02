#include "ScoreComponent.h"
#include "Event.h"
#include "GameObject.h"

void dae::ScoreComponent::Update()
{}

void dae::ScoreComponent::Render()
{}

void dae::ScoreComponent::AddScore(int amount)
{
	m_Score += amount;

	Event event{ "ScoreChanged" };
	event.nbArgs = 1;
	event.args[0].intValue = m_Score;

	m_Subject.NotifyObservers(event, GetOwner());
}
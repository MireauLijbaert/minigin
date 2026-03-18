#include "HealthComponent.h"
#include "Event.h"
#include "GameObject.h"

void dae::HealthComponent::Update()
{
	// Nothing needed for now
}

void dae::HealthComponent::Render()
{
	// Nothing needed for now
}

void dae::HealthComponent::TakeDamage(int damage)
{
	m_CurrentHealth -= damage;

	if (m_CurrentHealth < 0)
	{
		m_CurrentHealth = 0;
	}

	// 
	Event event{ "LifeChanged" };

	event.args[0] = EventArg{ .intValue = m_CurrentHealth };

	m_Subject.NotifyObservers(event, GetOwner());
}

void dae::HealthComponent::Heal(int healAmount)
{
	m_CurrentHealth += healAmount;

	if (m_CurrentHealth > m_MaxHealth)
	{
		m_CurrentHealth = m_MaxHealth;
	}

	// Fire same event (UI just cares that it changed)
	Event event{ "LifeChanged" };

	m_Subject.NotifyObservers(event, GetOwner());
}
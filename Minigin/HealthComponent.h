#pragma once
#include "BaseComponent.h"
#include "Subject.h"

namespace dae
{
	class GameObject;

	class HealthComponent : public BaseComponent
	{
	public:

		HealthComponent(GameObject& pOwner, int maxHealth) : BaseComponent(pOwner), m_MaxHealth{ maxHealth }, m_CurrentHealth{ maxHealth } {}
		~HealthComponent() = default;
		HealthComponent(const HealthComponent& other) = delete;
		HealthComponent(HealthComponent&& other) = delete;
		HealthComponent& operator=(const HealthComponent& other) = delete;
		HealthComponent& operator=(HealthComponent&& other) = delete;

		void Update() override;
		void Render() override;

		void TakeDamage(int damage);
		void Heal(int healAmount);
		int GetCurrentHealth() const { return m_CurrentHealth; }
		int GetMaxHealth() const { return m_MaxHealth; }

		Subject& GetSubject() { return m_Subject; }


	private:
		// Use ints as we have a set amount of lives
		int m_MaxHealth;
		int m_CurrentHealth;
		Subject m_Subject;
	};
}

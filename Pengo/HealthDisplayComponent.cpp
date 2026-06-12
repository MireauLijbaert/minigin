#include "HealthDisplayComponent.h"
#include "Subject.h"
#include "TextComponent.h"
#include "Event.h"
#include "HealthComponent.h"

#include <string>

namespace dae
{
	HealthDisplayComponent::HealthDisplayComponent(GameObject& owner, HealthComponent* pHealthComponent, TextComponent* pTextComponent)
		: BaseComponent(owner)
		, m_HealthComponent{ pHealthComponent }
		, m_pTextComponent{ pTextComponent }
	{
		if (m_HealthComponent)
		{
			m_HealthComponent->GetSubject().AddObserver(this);
		}
	}

	HealthDisplayComponent::~HealthDisplayComponent()
	{
		if (m_HealthComponent)
		{
			m_HealthComponent->GetSubject().RemoveObserver(this);
		}
	}

	void HealthDisplayComponent::Update()
	{}

	void HealthDisplayComponent::Render()
	{}

	void HealthDisplayComponent::Notify(const Event& event, GameObject* actor)
	{
		actor; // unused for now

		if (event.id == "LifeChanged")
		{
			if (m_pTextComponent && event.nbArgs > 0)
			{
				const int currentHealth = event.args[0].intValue;
				m_pTextComponent->SetText("Lives: " + std::to_string(currentHealth));
			}
		}
	}

	void HealthDisplayComponent::OnSubjectDestroyed(Subject* subject)
	{
		if (&m_HealthComponent->GetSubject() == subject)
			m_HealthComponent = nullptr;
	}
}
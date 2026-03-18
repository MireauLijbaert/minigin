#include "HealthDisplayComponent.h"
#include "Subject.h"
#include "TextComponent.h"
#include "Event.h"

#include <string>

namespace dae
{
	HealthDisplayComponent::HealthDisplayComponent(GameObject& owner, Subject* pSubject, TextComponent* pTextComponent)
		: BaseComponent(owner)
		, m_pSubject{ pSubject }
		, m_pTextComponent{ pTextComponent }
	{
		if (m_pSubject)
		{
			m_pSubject->AddObserver(this);
		}
	}

	HealthDisplayComponent::~HealthDisplayComponent()
	{
		if (m_pSubject)
		{
			m_pSubject->RemoveObserver(this);
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
}
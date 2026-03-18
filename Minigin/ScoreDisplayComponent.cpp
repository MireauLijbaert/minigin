#include "ScoreDisplayComponent.h"
#include "Subject.h"
#include "TextComponent.h"
#include "Event.h"

#include <string>

namespace dae
{
	ScoreDisplayComponent::ScoreDisplayComponent(GameObject& owner, Subject* pSubject, TextComponent* pTextComponent)
		: BaseComponent(owner)
		, m_pSubject{ pSubject }
		, m_pTextComponent{ pTextComponent }
	{
		if (m_pSubject)
		{
			m_pSubject->AddObserver(this);
		}
	}

	ScoreDisplayComponent::~ScoreDisplayComponent()
	{
		if (m_pSubject)
		{
			m_pSubject->RemoveObserver(this);
		}
	}

	void ScoreDisplayComponent::Update()
	{}

	void ScoreDisplayComponent::Render()
	{}

	void ScoreDisplayComponent::Notify(const Event& event, GameObject* actor)
	{
		actor;

		if (event.id == "ScoreChanged")
		{
			if (m_pTextComponent && event.nbArgs > 0)
			{
				const int currentScore = event.args[0].intValue;
				m_pTextComponent->SetText("Score: " + std::to_string(currentScore));
			}
		}
	}
}
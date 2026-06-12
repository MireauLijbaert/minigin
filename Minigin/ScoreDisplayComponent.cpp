#include "ScoreDisplayComponent.h"
#include "Subject.h"
#include "TextComponent.h"
#include "Event.h"
#include "ScoreComponent.h"

#include <string>

namespace dae
{
	ScoreDisplayComponent::ScoreDisplayComponent(GameObject& owner, ScoreComponent* pScoreComponent, TextComponent* pTextComponent)
		: BaseComponent(owner)
		, m_pScoreComponent{ pScoreComponent }
		, m_pTextComponent{ pTextComponent }
	{
		if (m_pScoreComponent)
		{
			m_pScoreComponent->AddObserver(this);
		}
	}

	ScoreDisplayComponent::~ScoreDisplayComponent()
	{
		if (m_pScoreComponent)
		{
			m_pScoreComponent->RemoveObserver(this);
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

	void ScoreDisplayComponent::OnSubjectDestroyed(Subject* subject)
	{
		if (&m_pScoreComponent->GetSubject() == subject)
			m_pScoreComponent = nullptr;
	}
}
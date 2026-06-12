#pragma once
#include "BaseComponent.h"
#include "Observer.h"

namespace dae
{
	class Subject;
	class TextComponent;
	class ScoreComponent;
	struct Event;
	class GameObject;

	class ScoreDisplayComponent final : public BaseComponent, public Observer
	{
	public:
		ScoreDisplayComponent(GameObject& owner, ScoreComponent* pScoreComponent, TextComponent* pTextComponent);
		~ScoreDisplayComponent() override;

		ScoreDisplayComponent(const ScoreDisplayComponent& other) = delete;
		ScoreDisplayComponent(ScoreDisplayComponent&& other) = delete;
		ScoreDisplayComponent& operator=(const ScoreDisplayComponent& other) = delete;
		ScoreDisplayComponent& operator=(ScoreDisplayComponent&& other) = delete;

		void Update() override;
		void Render() override;

		void Notify(const Event& event, GameObject* actor) override;
		void OnSubjectDestroyed(Subject* subject) override;

	private:
		ScoreComponent* m_pScoreComponent;
		TextComponent* m_pTextComponent;
	};
}
#pragma once
#include "BaseComponent.h"
#include "Observer.h"

namespace dae
{
	class Subject;
	class TextComponent;
	struct Event;
	class GameObject;

	class ScoreDisplayComponent final : public BaseComponent, public Observer
	{
	public:
		ScoreDisplayComponent(GameObject& owner, Subject* pSubject, TextComponent* pTextComponent);
		~ScoreDisplayComponent() override;

		ScoreDisplayComponent(const ScoreDisplayComponent& other) = delete;
		ScoreDisplayComponent(ScoreDisplayComponent&& other) = delete;
		ScoreDisplayComponent& operator=(const ScoreDisplayComponent& other) = delete;
		ScoreDisplayComponent& operator=(ScoreDisplayComponent&& other) = delete;

		void Update() override;
		void Render() override;

		void Notify(const Event& event, GameObject* actor) override;

	private:
		Subject* m_pSubject;
		TextComponent* m_pTextComponent;
	};
}
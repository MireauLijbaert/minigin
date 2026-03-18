#pragma once
#include "BaseComponent.h"
#include "Observer.h"

namespace dae
{
	class Subject;
	class TextComponent;
	struct Event;
	class GameObject;

	class HealthDisplayComponent final : public BaseComponent, public Observer
	{
	public:
		HealthDisplayComponent(GameObject& owner, Subject* pSubject, TextComponent* pTextComponent);
		~HealthDisplayComponent() override;

		HealthDisplayComponent(const HealthDisplayComponent& other) = delete;
		HealthDisplayComponent(HealthDisplayComponent&& other) = delete;
		HealthDisplayComponent& operator=(const HealthDisplayComponent& other) = delete;
		HealthDisplayComponent& operator=(HealthDisplayComponent&& other) = delete;

		void Update() override;
		void Render() override;

		void Notify(const Event& event, GameObject* actor) override;

	private:
		Subject* m_pSubject;
		TextComponent* m_pTextComponent;
	};
}
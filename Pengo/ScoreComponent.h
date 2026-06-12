#pragma once
#include "BaseComponent.h"
#include "Subject.h"

namespace dae
{
	class GameObject;

	class ScoreComponent final : public BaseComponent
	{
	public:
		ScoreComponent(GameObject& owner)
			: BaseComponent(owner)
			, m_Score{ 0 }
		{}

		~ScoreComponent() override = default;
		ScoreComponent(const ScoreComponent& other) = delete;
		ScoreComponent(ScoreComponent&& other) = delete;
		ScoreComponent& operator=(const ScoreComponent& other) = delete;
		ScoreComponent& operator=(ScoreComponent&& other) = delete;

		void Update() override;
		void Render() override;

		void AddScore(int amount);
		int GetScore() const { return m_Score; }

		Subject& GetSubject() { return m_Subject; }

	private:
		int m_Score;
		Subject m_Subject;
	};
}
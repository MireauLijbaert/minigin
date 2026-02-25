#pragma once
#include "BaseComponent.h"
#include "TextComponent.h"

namespace dae
{
	class GameObject;

	class FpsComponent : public BaseComponent
	{
	public:

		FpsComponent(GameObject& pOwner, TextComponent* textComponent) : BaseComponent(pOwner), m_textComponent{ textComponent } {};
		~FpsComponent() = default;
		FpsComponent(const FpsComponent& other) = default;
		FpsComponent(FpsComponent&& other) = default;
		FpsComponent& operator=(const FpsComponent& other) = default;
		FpsComponent& operator=(FpsComponent&& other) = default;

		void Update() override;
		void Render() override;

	private:
		TextComponent* m_textComponent{};
		float m_delayUpdate{ 0 };
	};
}

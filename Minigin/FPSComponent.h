#pragma once
#include "BaseComponent.h"
#include "TextComponent.h"

namespace dae
{
	class GameObject;

	class FpsComponent : public BaseComponent
	{
	public:

		FpsComponent(GameObject& pOwner, TextComponent* textComponent) : BaseComponent(pOwner), m_TextComponent{ textComponent } {};
		~FpsComponent() = default;
		FpsComponent(const FpsComponent& other) = default;
		FpsComponent(FpsComponent&& other) = default;
		FpsComponent& operator=(const FpsComponent& other) = default;
		FpsComponent& operator=(FpsComponent&& other) = default;

		void Update() override;
		void Render() override;

	private:
		TextComponent* m_TextComponent{};
		float m_DelayUpdate{ 0 };
	};
}

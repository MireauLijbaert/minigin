#pragma once
#include "BaseComponent.h"
#include <memory>
#include "Texture2D.h"

namespace dae
{
	class GameObject;

	class RenderComponent : public BaseComponent
	{
	public:
		RenderComponent(GameObject& pOwner);
		~RenderComponent();
		RenderComponent(const RenderComponent& other) = default;
		RenderComponent(RenderComponent&& other) = default;
		RenderComponent& operator=(const RenderComponent& other) = default;
		RenderComponent& operator=(RenderComponent&& other) = default;

		void Update() override {};
		void Render() override;
		void SetTexture(const std::string& filename);
		void SetTexture(const std::shared_ptr<Texture2D>& texture);
		void SetSize(float w, float h) { m_width = w; m_height = h; }

	private:
		std::shared_ptr<Texture2D> m_texture{};
		float m_width{ 0.f };
		float m_height{ 0.f };
	};
}

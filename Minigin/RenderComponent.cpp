#include "RenderComponent.h"
#include "ResourceManager.h"
#include "Renderer.h"
#include "GameObject.h"

dae::RenderComponent::RenderComponent(GameObject& pOwner)
	: BaseComponent(pOwner)
{
}

dae::RenderComponent::~RenderComponent()
{
	m_texture.reset();
}

void dae::RenderComponent::Render()
{
	if (!m_texture)
	{
		return;
	} 

	const auto& pos{ GetOwner()->GetWorldPosition().GetPosition()};
	dae::Renderer::GetInstance().RenderTexture(*m_texture, pos.x, pos.y);
}

void dae::RenderComponent::SetTexture(const std::string& filename)
{
	m_texture = ResourceManager::GetInstance().LoadTexture(filename);
}

void dae::RenderComponent::SetTexture(const std::shared_ptr<Texture2D>& texture)
{
	m_texture = texture;
}

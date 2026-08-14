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
	if (!m_visible || !m_texture) return;

	SDL_Texture* tex = m_texture->GetSDLTexture();
	if (!tex) return;

	const auto& pos = GetOwner()->GetWorldPosition().GetPosition();

	
	if (!m_hasSrcRect && !m_flipH && m_r == 255 && m_g == 255 && m_b == 255)
	{
		if (m_width > 0.f && m_height > 0.f)
			dae::Renderer::GetInstance().RenderTexture(*m_texture, pos.x, pos.y, m_width, m_height);
		else
			dae::Renderer::GetInstance().RenderTexture(*m_texture, pos.x, pos.y);
		return;
	}

	
	const bool tinted = (m_r != 255 || m_g != 255 || m_b != 255);
	if (tinted) SDL_SetTextureColorMod(tex, m_r, m_g, m_b);

	SDL_FRect dst{ pos.x, pos.y, m_width, m_height };
	const SDL_FRect* src = m_hasSrcRect ? &m_srcRect : nullptr;

	if (m_flipH)
		SDL_RenderTextureRotated(dae::Renderer::GetInstance().GetSDLRenderer(),
		                         tex, src, &dst, 0.0, nullptr, SDL_FLIP_HORIZONTAL);
	else
		SDL_RenderTexture(dae::Renderer::GetInstance().GetSDLRenderer(), tex, src, &dst);

	if (tinted) SDL_SetTextureColorMod(tex, 255, 255, 255);
}

void dae::RenderComponent::SetTexture(const std::string& filename)
{
	m_texture = ResourceManager::GetInstance().LoadTexture(filename);
}

void dae::RenderComponent::SetTexture(const std::shared_ptr<Texture2D>& texture)
{
	m_texture = texture;
}

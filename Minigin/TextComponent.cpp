#include <stdexcept>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "TextComponent.h"

#include "GameObject.h"
#include "Renderer.h"

using namespace dae;

TextComponent::TextComponent(GameObject& pOwner, const std::string& text, std::shared_ptr<Font> font)
	:BaseComponent(pOwner), m_textTexture(nullptr), m_font{ font }, m_text{ text }, m_needsUpdate(true)
{
}

TextComponent::~TextComponent()
{
	m_textTexture.reset();
	m_font.reset();
}

void TextComponent::Update()
{
	if (m_needsUpdate)
	{
		const SDL_Color color = { 255,255,255,255 }; // only white text
		const auto surf = TTF_RenderText_Blended(m_font->GetFont(), m_text.c_str(), m_text.length(), color);
		if (surf == nullptr)
		{
			throw std::runtime_error(std::string("Render text failed: ") + SDL_GetError());
		}
		auto texture = SDL_CreateTextureFromSurface(Renderer::GetInstance().GetSDLRenderer(), surf);
		if (texture == nullptr)
		{
			throw std::runtime_error(std::string("Create text texture from surface failed: ") + SDL_GetError());
		}
		SDL_DestroySurface(surf);
		m_textTexture = std::make_shared<Texture2D>(texture);
		m_needsUpdate = false;
	}
}

void TextComponent::Render()
{
	if (!m_textTexture)
	{
		return;
	}

	const auto& pos = GetOwner()->GetPosition().GetPosition(); // Same as RenderComponent CHANGE THIS
	Renderer::GetInstance().RenderTexture(*m_textTexture, pos.x, pos.y);
}

// This implementation uses the "dirty flag" pattern
void TextComponent::SetText(const std::string& text)
{
	m_text = text;
	m_needsUpdate = true;
}

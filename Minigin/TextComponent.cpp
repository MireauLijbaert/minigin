#include <stdexcept>
#include "TextComponent.h"
#include "GameObject.h"
#include <SDL3_ttf/SDL_ttf.h>
#include "Renderer.h"

using namespace dae;

TextComponent::TextComponent(GameObject& pOwner, RenderComponent* renderComponent ,const std::string& text, std::shared_ptr<Font> font)
	:BaseComponent(pOwner), m_renderComponent{renderComponent}, m_textTexture(nullptr), m_font{font}, m_text{text}, m_needsUpdate(true)
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
		const auto surf = TTF_RenderText_Blended(m_font->GetFont(), m_text.c_str(), m_text.length(), m_color);
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
		if (m_textTexture)
		{
			m_renderComponent->SetTexture(m_textTexture);
		}
		m_needsUpdate = false;
	}
}

void TextComponent::Render()
{
	// Has to be here for build, but doesn't need to do anything as the render component will handle rendering the text, will figure out how to abstract this away later
}

// This implementation uses the "dirty flag" pattern
void TextComponent::SetText(const std::string& text)
{
	m_text = text;
	m_needsUpdate = true;
}

// Added a method to set the color of the text, will need to re-render the text when this is called so set m_needsUpdate to true
void TextComponent::SetColor(const SDL_Color& color)
{ 
	m_color = color;
	m_needsUpdate = true;
} 
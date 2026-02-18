#pragma once
#include "BaseComponent.h"
#include <string>
#include <memory>
#include "Font.h"
#include "Texture2D.h"

namespace dae
{
	class TextComponent : public BaseComponent
	{
	public:
		//Constructor
		TextComponent(GameObject& pOwner, const std::string& text, std::shared_ptr<Font> font);

		//Destructor
		virtual ~TextComponent() override;

		//Rule of 5
		TextComponent(const TextComponent& other) = delete;
		TextComponent(TextComponent&& other) = delete;
		TextComponent& operator=(const TextComponent& other) = delete;
		TextComponent& operator=(TextComponent&& other) = delete;

		virtual void Update() override;
		virtual void Render() override;

		virtual void SetText(const std::string& text);
	private:

		bool m_needsUpdate;
		std::string m_text;
		std::shared_ptr<Font> m_font;
		std::shared_ptr<Texture2D> m_textTexture;
	};
}




#pragma once
#include "BaseComponent.h"
#include <memory>
#include <cstdint>
#include "Texture2D.h"
#include <SDL3/SDL.h>

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

		void Update() override {}
		void Render() override;

		void SetTexture(const std::string& filename);
		void SetTexture(const std::shared_ptr<Texture2D>& texture);
		void SetSize(float w, float h) { m_width = w; m_height = h; }

		// Draw only a sub-region of the texture (e.g. one frame of a sprite strip).
		void SetSourceRect(float x, float y, float w, float h)
		{
			m_srcRect     = { x, y, w, h };
			m_hasSrcRect  = true;
		}
		void ClearSourceRect() { m_hasSrcRect = false; }

		// Flip the rendered image horizontally.
		void SetFlipH(bool flip) { m_flipH = flip; }

		// Optional RGB tint (255,255,255 = no tint). Applied and restored each frame.
		void SetColorMod(uint8_t r, uint8_t g, uint8_t b) { m_r = r; m_g = g; m_b = b; }

	private:
		std::shared_ptr<Texture2D> m_texture{};
		float     m_width     { 0.f };
		float     m_height    { 0.f };
		bool      m_hasSrcRect{ false };
		SDL_FRect m_srcRect   {};
		bool      m_flipH     { false };
		uint8_t   m_r{ 255 }, m_g{ 255 }, m_b{ 255 };
	};
}

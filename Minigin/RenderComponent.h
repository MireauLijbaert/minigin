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

	private:

		std::shared_ptr<Texture2D> m_texture{};
	};
}

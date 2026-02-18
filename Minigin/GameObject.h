#pragma once
#include <string>
#include <memory>
#include "Transform.h"
#include "BaseComponent.h"

namespace dae
{
	class Texture2D;
	class GameObject 
	{
		Transform m_transform{};
		std::shared_ptr<Texture2D> m_texture{};
	public:
		// Game loop
		virtual void Update();
		virtual void Render() const;

		// Component management
		template <typename T>
		void AddComponent(std::unique_ptr<T> component);

		template <typename T>
		void RemoveComponent();

		template <typename T>
		T* GetComponent();

		template <typename T>
		bool HasComponent() const;

		void SetTexture(const std::string& filename);
		void SetPosition(float x, float y);
		dae::Transform GetPosition() const;

		GameObject() = default;
		virtual ~GameObject();
		GameObject(const GameObject& other) = delete;
		GameObject(GameObject&& other) = delete;
		GameObject& operator=(const GameObject& other) = delete;
		GameObject& operator=(GameObject&& other) = delete;

	private:
		// Members
		std::vector<std::unique_ptr<BaseComponent>> m_components{};
		std::vector<BaseComponent*> m_componentsToRemove{};

	};
}

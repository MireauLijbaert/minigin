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

// Component management (needs to be in header for templates)

template <typename T>
void dae::GameObject::AddComponent(std::unique_ptr<T> component)
{
	static_assert(std::is_base_of<BaseComponent, T>::value, "T must derive from BaseComponent");
	m_components.emplace_back(std::move(component));
}

template <typename T>
void dae::GameObject::RemoveComponent()
{
	for (auto it = m_components.begin(); it != m_components.end(); ++it)
	{
		if (dynamic_cast<T*>(it->get()))
		{
			m_components.erase(it);
			return;
		}
	}
}

template <typename T>
T* dae::GameObject::GetComponent()
{
	for (const std::unique_ptr<BaseComponent>& component : m_components)
	{
		if (auto* castedComponent = dynamic_cast<T*>(component.get()))
		{
			return castedComponent;
		}
	}
	return nullptr;
}

template <typename T>
bool dae::GameObject::HasComponent() const
{
	for (const auto& component : m_components)
	{
		if (dynamic_cast<T*>(component.get()))
		{
			return true;
		}
	}
	return false;
}

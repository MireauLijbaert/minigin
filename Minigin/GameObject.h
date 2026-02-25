#pragma once
#include <string>
#include <memory>
#include "Transform.h"
#include "BaseComponent.h"

namespace dae
{
	class Texture2D;
	class GameObject final
	{
		Transform m_transform{}; // Should be removed for component based rendering, but will keep for now to avoid refactoring too much at once, will remove in the future
	public:
		// Game loop
		void Update();
		void Render() const;
		void MarkForRemoval();
		bool IsMarkedForRemoval() const { return m_markedForRemoval; }

		// Component management
		template <typename T>
		void AddComponent(std::unique_ptr<T> component);

		template <typename T>
		void RemoveComponent();

		template <typename T>
		T* GetComponent();

		template <typename T>
		bool HasComponent() const;

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
		bool m_markedForRemoval{ false };
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
	// Remove all components of type T, normally i should only have one, components also don't require mark for deletion (teacher's advice)
	m_components.erase(
		std::remove_if(
			m_components.begin(),
			m_components.end(),
			[](const std::unique_ptr<BaseComponent>& component)
			{
				return dynamic_cast<T*>(component.get()) != nullptr;
			}
		),
		m_components.end()
	);
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

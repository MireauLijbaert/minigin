#include <string>
#include "GameObject.h"
#include "ResourceManager.h"
#include "Renderer.h"

dae::GameObject::~GameObject() = default;

void dae::GameObject::Update()
{
}

void dae::GameObject::Render() const
{
	const auto& pos = m_transform.GetPosition();
	Renderer::GetInstance().RenderTexture(*m_texture, pos.x, pos.y);
}

void dae::GameObject::SetTexture(const std::string& filename)
{
	m_texture = ResourceManager::GetInstance().LoadTexture(filename);
}

void dae::GameObject::SetPosition(float x, float y)
{
	m_transform.SetPosition(x, y, 0.0f);
}

dae::Transform dae::GameObject::GetPosition() const
{
	return m_transform;
}

// Component management

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


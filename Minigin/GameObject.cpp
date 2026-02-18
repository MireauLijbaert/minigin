#include <string>
#include "GameObject.h"
#include "ResourceManager.h"
#include "Renderer.h"

dae::GameObject::~GameObject() = default;

void dae::GameObject::Update()
{
	for (const auto& component : m_components)
	{
		component->Update();
	}
	// Remove components that were marked for removal
	for (const auto& component : m_componentsToRemove)
	{
		m_components.erase(std::remove_if(m_components.begin(), m_components.end(),
			[component](const std::unique_ptr<BaseComponent>& ptr) { return ptr.get() == component; }),
			m_components.end());
	}
	m_componentsToRemove.clear();
}

void dae::GameObject::Render() const
{
	const auto& pos = m_transform.GetPosition();
	Renderer::GetInstance().RenderTexture(*m_texture, pos.x, pos.y);

	for (const auto& component : m_components)
	{
		component->Render();
	}
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


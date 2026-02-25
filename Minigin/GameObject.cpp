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
}

void dae::GameObject::Render() const
{

	for (const auto& component : m_components)
	{
		component->Render();
	}
}

void dae::GameObject::MarkForRemoval()
{
	m_markedForRemoval = true;
}


void dae::GameObject::SetPosition(float x, float y)
{
	m_transform.SetPosition(x, y, 0.0f);
}

dae::Transform dae::GameObject::GetPosition() const
{
	return m_transform;
}


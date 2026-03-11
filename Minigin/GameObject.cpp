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


dae::GameObject* dae::GameObject::GetParent()
{
	return m_parent;
}

void dae::GameObject::SetParent(GameObject* parent, bool keepWorldPosition)
{
	// Do 5 things
	// 1. Check validity of new parent (can't be self, can't be a child of this object) or if parent is already the parent of this object
	if (parent == this || m_parent == parent || (parent && std::find(m_children.begin(), m_children.end(), parent) != m_children.end()))
	{
		return;
	}

	// 2. Update position, rotation and scale
	if (parent == nullptr)
	{
		SetLocalPosition(GetWorldPosition());
	}
	else
	{
		if (keepWorldPosition)
		{
			// Could make an operator overload of this later on to do it like it is in the slides
			//m_localPos = parent->GetWorldPosition() + m_localPosition;
			Transform localPos{};
			localPos.SetPosition(GetWorldPosition().GetPosition() - parent->GetWorldPosition().GetPosition());
			SetLocalPosition(localPos);
		}
		SetPositionDirty();
		
	}

	// 3. Remove itself from previous parent (if parent is not null)
	if (m_parent)
	{
		m_parent->RemoveChild(this);
	}

	// 4. Set the given parent on itself (this can be nullptr)
	m_parent = parent;

	// 5. Add itself as a child to the given parent (if parent is not null)
	if (parent)
	{
		parent->AddChild(this);
	}
}

size_t dae::GameObject::GetChildCount() const
{
	return m_children.size();
}

dae::GameObject* dae::GameObject::GetChildAt(int index) const
{
	return m_children.at(index);
}

// Position management

void dae::GameObject::SetLocalPosition(float x, float y)
{
	// 2D space, Z is always 0
	m_localPosition.SetPosition(x, y, 0.0f);
	SetPositionDirty();
}

void dae::GameObject::SetLocalPosition(const Transform& localPosition)
{
	SetLocalPosition(localPosition.GetPosition().x, localPosition.GetPosition().y);
}

void dae::GameObject::SetLocalPosition(const glm::vec3& localPosition)
{
	SetLocalPosition(localPosition.x, localPosition.y);
}

dae::Transform dae::GameObject::GetLocalPosition() const
{
	return m_localPosition;
}

dae::Transform dae::GameObject::GetWorldPosition()
{
	if (m_isPositionDirty)
		UpdateWorldPosition();
	return m_worldPosition;
}

// Private Parent-Child Functions
void dae::GameObject::AddChild(GameObject* child)
{
	// Check if child is already a child of this object, if so return
	if (std::find(m_children.begin(), m_children.end(), child) != m_children.end())
	{
		return;
	}
	// Doesn't need the other checks as this is only used in SetParent which already checks for validity of the parent
	m_children.push_back(child);
}

void dae::GameObject::RemoveChild(GameObject* child)
{
	// Remove child, if child is not a child of this object, nothing happens,
	// doesn't need checks as this is only used in SetParent which already checks for validity of the parent
	m_children.erase(
		std::remove(m_children.begin(), m_children.end(), child),
		m_children.end()
	);
}

void dae::GameObject::UpdateWorldPosition()
{
	if (m_isPositionDirty)
	{
		if (m_parent == nullptr)
		{
			m_worldPosition = m_localPosition;
		}
		else
		{
			m_worldPosition.SetPosition(m_parent->GetWorldPosition().GetPosition() + m_localPosition.GetPosition());
			// Could make an operator overload of this later on to do it like it is in the slides
			//m_worldPosition = m_parent->GetWorldPosition() + m_localPosition;
		}
		m_isPositionDirty = false;
	}
}

void dae::GameObject::SetPositionDirty()
{
	m_isPositionDirty = true;
	for (auto child : m_children)
	{
		child->SetPositionDirty();
	}
}
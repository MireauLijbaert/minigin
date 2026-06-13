#include <algorithm>
#include "Scene.h"

using namespace dae;

void Scene::Add(std::unique_ptr<GameObject> object)
{
	assert(object != nullptr && "Cannot add a null GameObject to the scene.");
	// If we're mid-update, defer so we don't invalidate the iterator
	if (m_isUpdating)
		m_pendingAdd.emplace_back(std::move(object));
	else
		m_objects.emplace_back(std::move(object));
}

void Scene::Remove(GameObject& object)
{
	object.MarkForRemoval();
	for (int i = 0; i < object.GetChildCount(); ++i)
	{
		object.GetChildAt(i)->MarkForRemoval();
	}
}

void Scene::RemoveAll()
{
	m_objects.clear();
	m_pendingAdd.clear();
}

void Scene::Update()
{
	m_isUpdating = true;
	for(auto& object : m_objects)
	{
		object->Update();
	}
	m_isUpdating = false;

	// Remove objects that are marked for removal
	m_objects.erase(
		std::remove_if(
			m_objects.begin(),
			m_objects.end(),
			[](const std::unique_ptr<GameObject>& obj)
			{
				return obj->IsMarkedForRemoval();
			}
		),
		m_objects.end()
	);

	// Flush objects added mid-update (e.g. egg-hatched sno-bees)
	for (auto& obj : m_pendingAdd)
		m_objects.emplace_back(std::move(obj));
	m_pendingAdd.clear();
}

void Scene::Render() const
{
	for (const auto& object : m_objects)
	{
		object->Render();
	}
}


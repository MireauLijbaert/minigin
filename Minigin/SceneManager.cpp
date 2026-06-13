#include "SceneManager.h"
#include "Scene.h"

void dae::SceneManager::ClearAll()
{
	m_scenes.clear();
}

void dae::SceneManager::RequestLoad(std::function<void()> loadFn)
{
	m_pendingLoad = std::move(loadFn);
}

void dae::SceneManager::Update()
{
	// Execute any pending level load at the start of a frame (safe: nothing is mid-Update yet)
	if (m_pendingLoad)
	{
		m_scenes.clear();
		auto fn = std::move(m_pendingLoad);
		fn();
		return; // new scenes update next frame
	}

	for(auto& scene : m_scenes)
	{
		scene->Update();
	}
}

void dae::SceneManager::Render()
{
	for (const auto& scene : m_scenes)
	{
		scene->Render();
	}
}

dae::Scene& dae::SceneManager::CreateScene()
{
	m_scenes.emplace_back(new Scene());
	return *m_scenes.back();
}

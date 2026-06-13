#pragma once
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include "Scene.h"
#include "Singleton.h"

namespace dae
{
	class Scene;
	class SceneManager final : public Singleton<SceneManager>
	{
	public:
		Scene& CreateScene();
		void ClearAll();
		void RequestLoad(std::function<void()> loadFn);

		void Update();
		void Render();
	private:
		friend class Singleton<SceneManager>;
		SceneManager() = default;
		std::vector<std::unique_ptr<Scene>> m_scenes{};
		std::function<void()> m_pendingLoad;
	};
}

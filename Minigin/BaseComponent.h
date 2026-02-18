#pragma once


namespace dae
{
	class GameObject;

	class BaseComponent
	{

	public:

		BaseComponent(GameObject& pOwner) : m_pOwner(&pOwner) {}
		virtual ~BaseComponent() = default;
		BaseComponent(const BaseComponent& other) = delete;
		BaseComponent(BaseComponent&& other) = delete;
		BaseComponent& operator=(const BaseComponent& other) = delete;
		BaseComponent& operator=(BaseComponent&& other) = delete;

		virtual void Update() = 0;
		virtual void Render() = 0;

	protected:

		GameObject* GetOwner() const { return m_pOwner; }

	private:
		GameObject* m_pOwner;
	};
}
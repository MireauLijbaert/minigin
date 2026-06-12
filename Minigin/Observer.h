#pragma once
namespace dae
{
	struct Event;
	class GameObject;
	class Subject;

	class Observer
	{
	public:
		virtual ~Observer() = default;
		virtual void Notify(const Event& event, GameObject* actor) = 0;
		virtual void OnSubjectDestroyed(Subject*) {}
	};
}

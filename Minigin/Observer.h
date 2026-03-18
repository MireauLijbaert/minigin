class Event;
class GameObject;

class Observer
{
public:
	virtual ~Observer() = default;
	virtual void Notify(const Event& event, GameObject* actor) = 0;
};
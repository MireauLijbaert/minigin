#pragma once
#include <vector>
namespace dae
{
    class Observer;
    struct Event;
    class GameObject;

    class Subject
    {
    public:
        void AddObserver(Observer* observer);
        void RemoveObserver(Observer* observer);
        void NotifyObservers(const Event& event, GameObject* actor);

    private:
        std::vector<Observer*> m_Observers{};
    };
}

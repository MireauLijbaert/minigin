#include "Subject.h"
#include "Observer.h"
#include <algorithm>

void dae::Subject::AddObserver(Observer* observer)
{
    if (!observer) return;

    if (std::find(m_Observers.begin(), m_Observers.end(), observer) == m_Observers.end())
    {
        m_Observers.push_back(observer);
    }
}

void dae::Subject::RemoveObserver(Observer* observer)
{
    m_Observers.erase(
        std::remove(m_Observers.begin(), m_Observers.end(), observer),
        m_Observers.end()
    );
}

void dae::Subject::NotifyObservers(const Event& event, GameObject* actor)
{
    for (Observer* observer : m_Observers)
    {
        if (observer)
        {
            observer->Notify(event, actor);
        }
    }
}
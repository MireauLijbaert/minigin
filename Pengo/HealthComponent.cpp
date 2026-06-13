#include "HealthComponent.h"
#include "Event.h"
#include "GameObject.h"

void dae::HealthComponent::LoseLife()
{
    if (!IsAlive()) return;

    --m_Lives;

    Event event{ "LifeChanged" };
    event.nbArgs = 1;
    event.args[0] = EventArg{ .intValue = m_Lives };
    m_Subject.NotifyObservers(event, GetOwner());
    // Respawn vs game-over is handled by whoever observes the LifeChanged event
}

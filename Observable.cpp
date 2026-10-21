#include "Observable.h"
#include "Observer.h"

#include <algorithm>

void Observable::attach(Observer *observer)
{
    if (observer == nullptr)
    {
        return;
    }
    // Only add it if it isn't in the list yet.
    if (std::find(observers.begin(), observers.end(), observer) == observers.end())
    {
        observers.push_back(observer);
    }
}

void Observable::detach(Observer *observer)
{
    observers.erase(std::remove(observers.begin(), observers.end(), observer),
                    observers.end());
}

void Observable::notify()
{
    // Loop over a copy, in case an observer detaches itself during update().
    std::vector<Observer *> current = observers;
    for (Observer *observer : current)
    {
        observer->update();
    }
}

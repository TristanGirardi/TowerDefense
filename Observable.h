#ifndef OBSERVABLE_H
#define OBSERVABLE_H

#include <vector>

class Observer;

/**
 * @brief Anything that can be watched (Tower, Map, CritterGroup).
 *
 * Keeps a list of observers and calls update() on each of them when
 * notify() is called.
 */
class Observable
{
    public:
        Observable() = default;
        virtual ~Observable() = default;

    
        Observable(const Observable &) {}
        
        Observable &operator=(const Observable &) { return *this; }

        /** @brief Starts watching. */
        void attach(Observer *observer);

        /** @brief Stops watching. */
        void detach(Observer *observer);

    protected:
        /** @brief Tells every attached observer that something changed. */
        void notify();

    private:
        std::vector<Observer *> observers;
};

#endif
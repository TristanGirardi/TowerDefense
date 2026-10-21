#ifndef OBSERVER_H
#define OBSERVER_H


 //@brief Anything that wants to be told when an Observable changes.
 
 // The views (TowerObserver, MapObserver, CritterObserver) inherit from this and redraw themselves in update().
 
class Observer {
    public:
        virtual ~Observer() = default;

        // @brief Called by the Observable every time it changes. 
        virtual void update() = 0;
};

#endif
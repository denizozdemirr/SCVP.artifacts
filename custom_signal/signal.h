#ifndef SIGNAL_H
#define SIGNAL_H

#include <systemc.h>

template <class T>
class SignalInterface : public sc_interface
{
    public:
    virtual T read() = 0;
    virtual void write(T) = 0;
};

template <class T>
class Signal : public SignalInterface<T>, public sc_prim_channel
//prim channels allow us to put somethings in the SENSITIVITY LIST
//to add something to sensitivity list: 
//we must implement : UPDATE function, READ, and WRITE functions
{
    private:
    T currentValue;
    T newValue;
    sc_event valueChangedEvent; //event that will be put in the sensitivity list

    public:
    Signal() {
        currentValue = 0;
        newValue = 0;
    }

    T read() //MUST IMPLEMENT
    {
        return currentValue;
    }

    void write(T d)
    {
        newValue = d; //store the value we want to write in a newValue
        if(newValue != currentValue) //CHECK, if something has changed
        {
            request_update(); // Call to SystemC Scheudler
            //to notify that our signal has changes
        }
    }

    void update() //now implement, the UPDATE FUNC itself
    {
        if(newValue != currentValue) //if new value is different from current value;
        {
            currentValue = newValue; // current value will be stored in new value
            valueChangedEvent.notify(SC_ZERO_TIME); //THEN name of the event.NOTIFY
            //THAT'S HOW YOU IMPLEMENT EVENT
        }
    }

    //every time something has changed, this event is notified and all processes
    //that are sensitive to this EVENT, will also execute
    const sc_event& default_event() const {
        return valueChangedEvent; //RETURN THE POINTER to this event
    }
};
#endif // SIGNAL_H


//TAKEAWAYS for custom primitive channels like SIGNAL
// 1) return valueChangedEvent
// 2) implement an UPDATE function
// 3)const sc_event& default_event() do this if you want to use this signal
// in the sensitivity list
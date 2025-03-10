/*
 * Copyright 2017 Matthias Jung
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors:
 *     - Matthias Jung
 */


#include <iostream>
#include <systemc.h> 

using namespace std;

SC_MODULE(eventTester) // module defined
{
    sc_event triggerEvent; //said we have an event, look out

    SC_CTOR(eventTester) //2. olarak we have SC_ctor inside sc_module
    {
        SC_THREAD(triggerProcess); // sctor içinde sc_thread ve sc_module var
        SC_METHOD(sensitiveProcess);
        sensitive << triggerEvent; // sensitive list varsa o 
        dont_initialize();
    }

    void triggerProcess() // after sc_ctor, but again inside sc_module, we will define our function with void
    {
        // WANT TO TRIGGER IT ONCES
       wait(SC_ZERO_TIME); // this is a thread because we have WAIT
       triggerEvent.notify(10,SC_NS);
       triggerEvent.notify(20,SC_NS); // Will be ignored
       triggerEvent.notify(30,SC_NS); // Will be ignored
    }

    void sensitiveProcess() 
    {
        cout << "EVENT: @" << sc_time_stamp() << endl;
    }
};

// WANT TO TRIGGER MORE THAN ONCE = I USE QUEUE
SC_MODULE(eventQueueTester)
{
    sc_event_queue triggerEventQueue;

    SC_CTOR(eventQueueTester)
    {
        SC_THREAD(triggerProcess);
        SC_METHOD(sensitiveProcess);
        sensitive << triggerEventQueue;
        dont_initialize();
    }

    void triggerProcess()
    {
       wait(100,SC_NS); //kernel will wait 100ns, when reached 100ns the below is executed
       triggerEventQueue.notify(10,SC_NS);
       triggerEventQueue.notify(20,SC_NS); // Will not be ignored
       triggerEventQueue.notify(40,SC_NS); // Will not be ignored
       triggerEventQueue.notify(30,SC_NS); // Will not be ignored
    }

    void sensitiveProcess()
    {
        cout << "QUEUE: @" << sc_time_stamp() << endl;
    }
};

int sc_main(int __attribute__((unused)) argc,
            char __attribute__((unused)) *argv[])
{
    eventTester et("et");
    eventQueueTester eqt("eqt");
    sc_start();
    return 0;
}

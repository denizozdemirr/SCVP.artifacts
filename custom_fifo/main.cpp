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
#include <queue>

//DONT FORGET -> you implement eveytiny producer consumer etc. Then, you bind everything
// in the "int sc_main" by instantiating the classes.
//for example PRODUCER p1("Producer1")

using namespace std;

//THIS IS INTERFACE : describes how we can communicate with FIFO => we have "read" and "write" METHOD
template <class T> //we want it to be template (not jsut int )
class SimpleFIFOInterface : public sc_interface //sc_interface needed to show its an interface
{
    public:
    virtual T read() = 0; //virtual for some reason
    virtual void write(T) = 0;
    // Just for Debug
    virtual void printFIFO()
    {
        cout << "Warning: Debug Function Not Implemented" << endl;
    }
};

//THIS IS CHANNEL
template <class T>
class SimpleFIFO : public SimpleFIFOInterface<T> //we DERIVE FIFO from our simple FIFO interface
{
    private:
    std::queue<T> fifo; //where we store the data
    sc_event writtenEvent; //triggered when something is WRITTEN
    sc_event readEvent; //need this to unlock consumer of produder
    unsigned int maxSize;

    public:
    SimpleFIFO(unsigned int size=16) : maxSize(size) //CONSTRUCTOR
    {
    }


    // read the fifo.pop() yaptık
    // write da fifo.push() yaptık

    T read() //READ is public again
    {
        if(fifo.empty() == true)
        {
            std::cout << "Wait for Write" << std::endl;
            wait(writtenEvent); // wait for written event, then it will be unlocked
        }
        T val = fifo.front(); //first element of the FIFO
        fifo.pop(); //pop the first value, yani READ THE VALUE
        readEvent.notify(SC_ZERO_TIME); //since its popped, we have read event
        return val; //so, we return the popped value 
    } 

    void write(T d) //write is VOID
    {
        if(fifo.size() == maxSize) // if the FIFO is FULL, means read it already
        {
            std::cout << "Wait for Read" << std::endl;
            wait(readEvent); //wait for read
        }
        fifo.push(d); //push our value to the FIFO, the value we want to write
        writtenEvent.notify(SC_ZERO_TIME);
    }

    void printFIFO()
    {
        unsigned int n = fifo.size();

        std::cout << "SimpleFIFO (" << maxSize << ") " << "[";
        for(unsigned int i = 0; i < n; i++) {
            std::cout << "█";
        }
        for(unsigned int i = 0; i < maxSize-n; i++) {
            std::cout << " ";
        }
        std::cout << "]" << std::endl;
        std::cout.flush();
    }
};

SC_MODULE(PRODUCER) //WRITES TO FIFO
{
    sc_port< SimpleFIFOInterface<int> > master; // port is described by our simple FIFO interface
    // port name is MASTER

    SC_CTOR(PRODUCER) // constructor has the same name as class
    {
        SC_THREAD(process); // THREAD because FIFO, and we have a function called PROCESS
    } 

    void process() //now, define this function 
    {
        while(true) // have WHILE loop because its THREAD
        {
            wait(1,SC_NS); // EVERY NANOSECOND, PRODUDER PRODUCES SOMETHING
            master->write(10); //we are writing to the master port !!!
            //dont use master.write(10) USE THIS !!! ->
            std::cout << "@" << sc_time_stamp() << " Write: 10 ";
            master->printFIFO(); 
        }
    }
};

SC_MODULE(CONSUMER)
{
    sc_port< SimpleFIFOInterface<int> > slave; //slave port 

    SC_CTOR(CONSUMER)
    {
        SC_THREAD(process);
        sensitive << slave;
    }

    void process()
    {
        while(true)
        {
            wait(4,SC_NS);
            std::cout << "@" << sc_time_stamp() << " Read : "
                      << slave->read() << " ";
            slave->printFIFO();
        }
    }
};

int sc_main(int __attribute__((unused)) argc,
            char __attribute__((unused)) *argv[])
{
    PRODUCER pro1("pro1"); 
    CONSUMER con1("con1");
    SimpleFIFO<int> channel(4);

    sc_signal<int> foo;

    pro1.master.bind(channel); // bind master and slave to channel
    con1.slave.bind(channel);

    sc_start(10,SC_NS); //simulate 10ns, then stop the simulation

    return 0;
}

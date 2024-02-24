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

#include <systemc.h>

SC_MODULE(rslatch) //  1)) MODULE for SR LATCH
{
    sc_in<bool> S;
    sc_in<bool> R;
    sc_out<bool> Q;
    sc_out<bool> N;

    SC_CTOR(rslatch) : S("S"), R("R"), Q("Q"), N("N") // 2)) inside the MODULE => CONSTRUCTOR
    {
        SC_METHOD(process);           // 3)) inside SC_CTOR => SC_METHOD
        sensitive << S << R << Q << N;
    }

    void process()                    // 4) after SC_METHOD, inside SC_CTOR => PROCESS
                                      // this is where we define the functionality, what our method should do.
    {
        // what it should do? We have 2 equations for SR latch, sadece onları yazdık burada
        Q.write(!(R.read()||N.read())); // Nor Gate
        N.write(!(S.read()||Q.read())); // Nor Gate
    }
};
// 1)SC_MODULE,  2)inside this module= SC_CTOR, 3)inside this SC_CTOR= either SC_THREAD or SC_METHOD
// 4)inside this constructor = tanımladığın functionların void ları. 5) int sc_main de de instantiation ları
SC_MODULE(toplevel) //!!!!!! MODULE FOR TESTBENCH basically 
{
    sc_signal<bool> Ssig; //with these signals,
    sc_signal<bool> Rsig; //we connect to a testbench
    sc_signal<bool> Qsig;
    sc_signal<bool> Nsig;

    rslatch rs; //instantiation of our sr latch yukarıda yaptığımız
    sc_time currentTime;
    unsigned long long currentDelta;

    SC_CTOR(toplevel) : rs("rs") //constructor
    {
        SC_THREAD(process); //inside constructor, we bind ports to signals

        rs.S(Ssig); //bind S to Ssig
        rs.R(Rsig);
        rs.Q(Qsig);
        rs.N(Nsig);

        std::cout << "\nS=0, R=1, Q=0, N=1\n" << std::endl;
        Ssig.write(false); //just giving some values to the signals
        Rsig.write(true);
        Qsig.write(false);
        Nsig.write(true);

        //these lines are just for debugging, to have an output and so on
        currentTime = SC_ZERO_TIME;
        currentDelta = sc_delta_count();
    }

    void waitAndPrint(sc_time delay)
    {
        wait(delay);
        sc_time time = sc_time_stamp();

        // The sc_delta_count() returns the total number of executed
        // delta delays. In order to estimate the delta delay
        // between the time advances the following is done:
        if(time > currentTime)
        {
           currentDelta = sc_delta_count();
           currentTime = time;
        }

        unsigned long long delta = sc_delta_count() - currentDelta;

        std::cout << time <<" + " << delta << "δ\t"
                  << Ssig.read() << "\t"
                  << Rsig.read() << "\t"
                  << Qsig.read() << "\t"
                  << Nsig.read() << "\t" << std::endl;
    }

    void process() //these processes just introduces some delta delays
    //JUST A TRICK TO MAKE THEM VIISBLE
    {
        // Start in Reset State
        waitAndPrint(SC_ZERO_TIME);
        waitAndPrint(SC_ZERO_TIME);
        waitAndPrint(SC_ZERO_TIME);

        // Set:
        std::cout << "\nS=1, R=0\n" << std::endl;
        waitAndPrint(sc_time(10,SC_NS));

        Ssig.write(true);
        Rsig.write(false);

        waitAndPrint(SC_ZERO_TIME);
        waitAndPrint(SC_ZERO_TIME);
        waitAndPrint(SC_ZERO_TIME);

        // Reset:
        std::cout << "\nS=0, R=1\n" << std::endl;
        waitAndPrint(sc_time(10,SC_NS));

        Ssig.write(false);
        Rsig.write(true);

        waitAndPrint(SC_ZERO_TIME);
        waitAndPrint(SC_ZERO_TIME);
        waitAndPrint(SC_ZERO_TIME);

        sc_stop();
    }
};


// in sc_main function, instantiate the test bench (which is top level),
// then sc_start(), return 0 and that's it.
int sc_main (int __attribute__((unused)) sc_argc,
             char __attribute__((unused)) *sc_argv[])
{
    std::cout << "\nT\t\tS\tR\tQ\tN" << std::endl;
    toplevel t("t"); //instance of the toplevel

    sc_set_stop_mode(SC_STOP_FINISH_DELTA);
    sc_start(); //run this

    return 0;
}

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
//we said we HAVE A MODULE CALLED NOT, we will use this module later for more than one not chain
// it is useful if we will use more than one not, combine them etc.
SC_MODULE(NOT) // we have 1) NOT module(SC_MODULE named NOT)
{
    public: //have to specifiy public and private things
    sc_in<bool> in; //define the signals inside public
    sc_out<bool> out;

    SC_CTOR(NOT) : in("in"), out("out") //after SC_MODULE, and inside rhis, we have SC_CTOR (aynı name li)
    // and we say "in" is an in port, out is an out port falan
    {
        SC_METHOD(process); // inside this SC_CTOR, we have a SC_METHOD called process,
        // we will define this process as void outside this SC_TOR, but again inside this SC_MODULE
    }

    void process() // just specify what this process should do
    { 
       out.write(!in.read()); // it will read() in, negate this value !, write() it to out
    }
};

SC_MODULE(not_chain) // another module called not_chain, where we'll use 
{
    sc_in<bool> A; //inide this module, define in and out
    sc_out<bool> Z;

    NOT not1, not2, not3; //instantiate the not instances, how many we need

    sc_signal<bool> h1,h2; // interconnect signals, they connect the nots 

    SC_CTOR(not_chain): //constructor, define every signal, interconnect, port
        A("A"), Z("Z"),
        not1("not1"), not2("not2"), not3("not3"),
        h1("h1"), h2("h2")
    {
        // THEN, BIND!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //~~~~~~~~~~~~~~~~
        //        h1    h2
        // A--NOT1--NOT2--NOT3--Z
        not1.in.bind(A);
        not1.out.bind(h1);

        not2.in(h1);
        not2.out(h2);

        not3.in(h2);
        not3.out(Z);
    }
};

// BURADA hep gerekli zaten
int sc_main (int __attribute__((unused)) sc_argc,
             char __attribute__((unused)) *sc_argv[])
{
    sc_signal<bool> foo, bar;

    not_chain c("not_chain");

    foo.write(0);
    c.A.bind(foo);
    c.Z.bind(bar);

    sc_start();

    std::cout << bar.read() << std::endl;
    return 0;
}

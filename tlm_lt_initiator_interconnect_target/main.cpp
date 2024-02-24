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
#include <tlm.h>

using namespace std;

class exampleInitiator: sc_module, tlm::tlm_bw_transport_if<> // INITIATOR CLASS !!!
{ 
    public:
    tlm::tlm_initiator_socket<> iSocket; //its not a port but a SOCKET NOW
    SC_CTOR(exampleInitiator) : iSocket("iSocket")
    {
        // bind the socket to the initiator module itself
        iSocket.bind(*this); //this points to the object of the class
        SC_THREAD(process);
    }

    // the functions below, we HAVE TO implement them because we are doing an
    // initiator, so for this interface, even if you dont use them, HAVE TO IMPLEMENT THEM
    // invalidate_direct_mem_ptr these things

    // Dummy method:
    void invalidate_direct_mem_ptr(sc_dt::uint64 start_range,
                                   sc_dt::uint64 end_range)
    {
        SC_REPORT_FATAL(this->name(),"invalidate_direct_mem_ptr not implement");
    }

    // Dummy method:
    tlm::tlm_sync_enum nb_transport_bw(
            tlm::tlm_generic_payload& trans,
            tlm::tlm_phase& phase,
            sc_time& delay)
    {
        SC_REPORT_FATAL(this->name(),"nb_transport_bw is not implemented");
        return tlm::TLM_ACCEPTED;
    }

    private:
    void process()
    {
        // Write to memory1:
        for (int i = 0; i < 4; i++) {
            tlm::tlm_generic_payload trans; //our transaction object 
            unsigned char data = rand();
            //HAVE TO SET THE FOLLOWING
            trans.set_address(i); //set the address to i like in the previous example
            trans.set_data_length(1); //set data length
            trans.set_streaming_width(1); //set the width
            trans.set_command(tlm::TLM_WRITE_COMMAND); //indicate which command it is 
            trans.set_data_ptr(&data); //to get the reference
            trans.set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );
            sc_time delay = sc_time(0, SC_NS);

            iSocket->b_transport(trans, delay);
            // can put here wait(delay)

            if ( trans.is_response_error() )
              SC_REPORT_FATAL(name(), "Response error from b_transport");
            wait(delay); //SOMETIMES, put this in the initiator meaning : look up
            std::cout << "@" << sc_time_stamp() << " Write Data: "
                      << (unsigned int)data << std::endl;
        }
        // Write to memory2:
        for (int i = 512; i < 515; i++) {
            tlm::tlm_generic_payload trans;
            unsigned char data = rand();
            trans.set_address(i);
            trans.set_data_length(1);
            trans.set_streaming_width(1);
            trans.set_command(tlm::TLM_WRITE_COMMAND);
            trans.set_data_ptr(&data);
            trans.set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );
            sc_time delay = sc_time(0, SC_NS);
            iSocket->b_transport(trans, delay);
            if ( trans.is_response_error() )
              SC_REPORT_FATAL(name(), "Response error from b_transport");
            wait(delay);
            std::cout << "@" << sc_time_stamp() << " Write Data: "
                      << (unsigned int)data << std::endl;

        }

        // Read from memory1:
        for (int i = 0; i < 4; i++) {
            tlm::tlm_generic_payload trans;
            unsigned char data;
            trans.set_address(i);
            trans.set_data_length(1);
            trans.set_streaming_width(1);
            trans.set_command(tlm::TLM_READ_COMMAND);
            trans.set_data_ptr(&data);
            trans.set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );
            sc_time delay = sc_time(0, SC_NS);
            iSocket->b_transport(trans, delay);
            if ( trans.is_response_error() )
              SC_REPORT_FATAL(name(), "Response error from b_transport");
            wait(delay);
            std::cout << "@" << sc_time_stamp() << " Read Data: "
                      << (unsigned int)data << std::endl;
        }
        // Read from memory2:
        for (int i = 512; i < 515; i++) {
            tlm::tlm_generic_payload trans;
            unsigned char data;
            trans.set_address(i);
            trans.set_data_length(1);
            trans.set_streaming_width(1);
            trans.set_command(tlm::TLM_READ_COMMAND);
            trans.set_data_ptr(&data);
            trans.set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );
            sc_time delay = sc_time(0, SC_NS);
            iSocket->b_transport(trans, delay);
            if ( trans.is_response_error() )
              SC_REPORT_FATAL(name(), "Response error from b_transport");
            wait(delay);
            std::cout << "@" << sc_time_stamp() << " Read Data: "
                      << (unsigned int)data << std::endl;
        }
    }
};

class exampleTarget : sc_module, tlm::tlm_fw_transport_if<> //TARGET CLASS !!!
{
    private:
    unsigned char mem[512];

    public:
    tlm::tlm_target_socket<> tSocket;

    SC_CTOR(exampleTarget) : tSocket("tSocket")
    {
        tSocket.bind(*this); //bind the socket to the target module
    }

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    void b_transport(tlm::tlm_generic_payload &trans, sc_time &delay) // MOST IMPORTANT
    {
        if (trans.get_address() >= 512)
        {
             trans.set_response_status( tlm::TLM_ADDRESS_ERROR_RESPONSE );
             return;
        }

        if (trans.get_data_length() != 1)
        {
             trans.set_response_status( tlm::TLM_BURST_ERROR_RESPONSE );
             return;
        }

        if(trans.get_command() == tlm::TLM_WRITE_COMMAND)
        {
            memcpy(&mem[trans.get_address()], // destination
                   trans.get_data_ptr(),      // source
                   trans.get_data_length());  // size
            //delay = delay + sc_time(20, SC_NS);
        }
        else // (trans.get_command() == tlm::TLM_READ_COMMAND)
        {
            memcpy(trans.get_data_ptr(),      // destination
                   &mem[trans.get_address()], // source
                   trans.get_data_length());  // size
            //delay = delay + sc_time(10, SC_NS);
        }

        delay = delay + sc_time(40, SC_NS); //delay
        //BUT, if for example WRITE needs more time than READ: look up

        trans.set_response_status( tlm::TLM_OK_RESPONSE );
    }

    // Dummy method
    virtual tlm::tlm_sync_enum nb_transport_fw(
            tlm::tlm_generic_payload& trans,
            tlm::tlm_phase& phase,
            sc_time& delay )
    {
        SC_REPORT_FATAL(this->name(),"nb_transport_fw is not implemented");
        return tlm::TLM_ACCEPTED;
    }

    // Dummy method
    bool get_direct_mem_ptr(tlm::tlm_generic_payload& trans,
                            tlm::tlm_dmi& dmi_data)
    {
        SC_REPORT_FATAL(this->name(),"get_direct_mem_ptr is not implemented");
        return false;
    }

    // Dummy method
    unsigned int transport_dbg(tlm::tlm_generic_payload& trans)
    {
        SC_REPORT_FATAL(this->name(),"transport_dbg is not implemented");
        return 0;
    }

};

class exampleInterconnect : sc_module,
                            tlm::tlm_bw_transport_if<>,
                            tlm::tlm_fw_transport_if<>
{
    public:
    tlm::tlm_initiator_socket<> iSocket[2];
    tlm::tlm_target_socket<> tSocket;

    SC_CTOR(exampleInterconnect)
    {
        tSocket.bind(*this);
        iSocket[0].bind(*this);
        iSocket[1].bind(*this);
    }

    void b_transport(tlm::tlm_generic_payload &trans, sc_time &delay)
    {
        // Annotate Bus Delay
        delay = delay + sc_time(40, SC_NS);

        if(trans.get_address() < 512)
        {
            iSocket[0]->b_transport(trans, delay);
        }
        else
        {
            // Correct Address:
            trans.set_address(trans.get_address() - 512);
            iSocket[1]->b_transport(trans, delay);
        }
    }

    // Dummy method
    virtual tlm::tlm_sync_enum nb_transport_fw(
            tlm::tlm_generic_payload& trans,
            tlm::tlm_phase& phase,
            sc_time& delay )
    {
        SC_REPORT_FATAL(this->name(),"nb_transport_fw is not implemented");
        return tlm::TLM_ACCEPTED;
    }

    // Dummy method
    bool get_direct_mem_ptr(tlm::tlm_generic_payload& trans,
                            tlm::tlm_dmi& dmi_data)
    {
        SC_REPORT_FATAL(this->name(),"get_direct_mem_ptr is not implemented");
        return false;
    }

    // Dummy method
    unsigned int transport_dbg(tlm::tlm_generic_payload& trans)
    {
        SC_REPORT_FATAL(this->name(),"transport_dbg is not implemented");
        return 0;
    }

    // Dummy method:
    void invalidate_direct_mem_ptr(sc_dt::uint64 start_range,
                                   sc_dt::uint64 end_range)
    {
        SC_REPORT_FATAL(this->name(),"invalidate_direct_mem_ptr not implement");
    }

    // Dummy method:
    tlm::tlm_sync_enum nb_transport_bw(
            tlm::tlm_generic_payload& trans,
            tlm::tlm_phase& phase,
            sc_time& delay)
    {
        SC_REPORT_FATAL(this->name(),"nb_transport_bw is not implemented");
        return tlm::TLM_ACCEPTED;
    }

};

int sc_main (int __attribute__((unused)) sc_argc,
             char __attribute__((unused)) *sc_argv[])
{

    exampleInitiator * cpu    = new exampleInitiator("cpu");
    exampleTarget * memory1   = new exampleTarget("memory1");
    exampleTarget * memory2   = new exampleTarget("memory2");
    exampleInterconnect * bus = new exampleInterconnect("bus");

    cpu->iSocket.bind(bus->tSocket);
    bus->iSocket[0].bind(memory1->tSocket);
    bus->iSocket[1].bind(memory2->tSocket);

    sc_start();

    return 0;
}

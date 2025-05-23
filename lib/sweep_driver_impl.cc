/* -*- c++ -*- */
/*
 * Copyright 2024 Jimmy Fitzpatrick.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */


#include "sweep_driver_impl.h"
#include <gnuradio/io_signature.h>

namespace gr {
namespace spectre {


using input_type = gr_complex;


sweep_driver::sptr sweep_driver::make(
    float min_freq, float max_freq, float freq_step, int samp_rate, int samples_per_step, std::string retune_cmd_name)
{
    return gnuradio::make_block_sptr<sweep_driver_impl>(
        min_freq, max_freq, freq_step, samp_rate, samples_per_step, retune_cmd_name);
}


sweep_driver_impl::sweep_driver_impl(
    float min_freq, float max_freq, float freq_step, int samp_rate, int samples_per_step, std::string retune_cmd_name)
    : gr::sync_block("sweep_driver",
                     gr::io_signature::make(1, 1, sizeof(input_type)),
                     gr::io_signature::make(0, 0, 0)),
    _min_freq(min_freq),
    _max_freq(max_freq),
    _freq_step(freq_step),
    _samp_rate(samp_rate),
    _samples_per_step(samples_per_step),
    _retune_cmd_name(pmt::string_to_symbol(retune_cmd_name)),
    _sample_count(0),
    _current_freq(_min_freq)
{   
    // Declare the message port in the constructor (https://wiki.gnuradio.org/index.php/Message_Passing)
    message_port_register_out( OUTPUT_PORT );
}


sweep_driver_impl::~sweep_driver_impl() {}


void sweep_driver_impl::publish_retune_command() 
{
    pmt::pmt_t retune_command = pmt::make_dict();
    retune_command = pmt::dict_add(retune_command, _retune_cmd_name, pmt::from_float(_current_freq));
    message_port_pub(OUTPUT_PORT, retune_command);
}


int sweep_driver_impl::work(int noutput_items,
                            gr_vector_const_void_star& input_items,
                            gr_vector_void_star& output_items)
{

    for (int i = 0; i < noutput_items; i++)
    {
        // Increment the sample count.
        _sample_count++;

        // Check if we have reached the final sample in the step.
        if (_sample_count == _samples_per_step) 
        {
            // If so, reset the sample count.
            _sample_count = 0;
            // Increment the output frequency of sweep driver.
            _current_freq += _freq_step;

            // If the incremented frequency is larger than the maximum frequency
            // imposed by the user, then reset it to the initial state.
            // In this way, we never publish a center frequency ABOVE
            // the user-configured maximum center frequency.
            if (_current_freq > _max_freq) 
            {
                _current_freq = _min_freq;
            }

            // Publish the command to retune the receiver.
            publish_retune_command();
        }
    }

    // Return the number of output items produced
    return noutput_items;
}


} /* namespace spectre */
} /* namespace gr */

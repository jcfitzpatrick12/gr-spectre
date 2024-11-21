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

// Key things to work out... try and decipher the message passing API.
// Why is the msg object I am publishing a pair? Is they key simply ignored
// by the receiving block?

sweep_driver::sptr sweep_driver::make(
    float min_freq, float max_freq, float freq_step, int samp_rate, int samples_per_step, std::string receiver_port_name)
{
    return gnuradio::make_block_sptr<sweep_driver_impl>(
        min_freq, max_freq, freq_step, samp_rate, samples_per_step, receiver_port_name);
}

sweep_driver_impl::sweep_driver_impl(
    float min_freq, float max_freq, float freq_step, int samp_rate, int samples_per_step, std::string receiver_port_name)
    : gr::sync_block("sweep_driver",
                     gr::io_signature::make(1, 1, sizeof(input_type)),
                     gr::io_signature::make(0, 0, 0)),
    _min_freq(min_freq),
    _max_freq(max_freq),
    _freq_step(freq_step),
    _samp_rate(samp_rate),
    _samples_per_step(samples_per_step),
    _receiver_port_name(pmt::string_to_symbol(receiver_port_name)),
    _initial_center_freq( compute_initial_center_freq() ),
    _sample_count(0),
    _current_freq(_initial_center_freq)
{   
    // Declare the message port in the constructor (https://wiki.gnuradio.org/index.php/Message_Passing)
    message_port_register_out( OUTPUT_PORT );
}

sweep_driver_impl::~sweep_driver_impl() {}

float sweep_driver_impl::compute_initial_center_freq() {
    // Compute the initial center frequency such that
    // on performing an FFT with the defined sampling rate, 
    // the mimimum frequency configured by the user will
    // coincide with the lowest frequency in the spectrum.
    return (_min_freq + _samp_rate / 2);
}

void sweep_driver_impl::publish_current_freq() {
    // Does the key have any effect at all?
    // Or is the destination port decided by flowgraph connections.
    pmt::pmt_t key = _receiver_port_name;
    pmt::pmt_t value = pmt::from_float(_current_freq);
    // cons - "construct a pair"
    pmt::pmt_t msg = pmt::cons(key, value);
    message_port_pub(OUTPUT_PORT, msg);
}

int sweep_driver_impl::work(int noutput_items,
                            gr_vector_const_void_star& input_items,
                            gr_vector_void_star& output_items)
{

    for (int i = 0; i < noutput_items; i++) {
        // Increment the sample count.
        _sample_count++;
        
        // Check if we have exceeded the range of the step.
        if (_sample_count > _samples_per_step) {
            // If so, reset the sample count.
            _sample_count = 0;

            // Increment the output frequency of sweep driver
            _current_freq += _freq_step;

            // If the incremented frequency is larger than the maximum frequency
            // imposed by the user, then reset it to the initial state.
            // In this way, we never publish a center frequency ABOVE
            // the user-configured maximum center frequency.
            if (_current_freq > _max_freq) {
                _current_freq = _initial_center_freq;
            }

            // Publish the frequency of the current step
            publish_current_freq();
        }
    }

    // Return the number of output items produced
    return noutput_items;
}



} /* namespace spectre */
} /* namespace gr */

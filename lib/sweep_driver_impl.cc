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
    _min_freq(min_freq), // set with user input
    _max_freq(max_freq), // set with user input
    _freq_step(freq_step), // set with user input
    _samp_rate(samp_rate), // set with user input
    _samples_per_step(samples_per_step), // set with user input
    _receiver_port_name(receiver_port_name), // set with user input
    _freq0(compute_freq0()), // compute based on user input
    _sample_index_within_step(0), // by convention, the initial sample index within the step is 0
    _current_freq(_freq0) // the current frequency is initially the computed _freq0
{   
    // declare an output message port for sweep driver
    message_port_register_out(pmt::mp("freq"));
}

sweep_driver_impl::~sweep_driver_impl() {}

float sweep_driver_impl::compute_freq0() {
    return (_min_freq + _samp_rate / 2);
}

void sweep_driver_impl::publish_current_freq() {
    pmt::pmt_t key = pmt::intern(_receiver_port_name);
    pmt::pmt_t value = pmt::from_float(_current_freq);
    pmt::pmt_t msg = pmt::cons(key, value);
    message_port_pub(pmt::mp("freq"), msg);
}

int sweep_driver_impl::work(int noutput_items,
                            gr_vector_const_void_star& input_items,
                            gr_vector_void_star& output_items)
{
    // Process each output item
    for (int i = 0; i < noutput_items; i++) {
        // Increment sample index within the step
        _sample_index_within_step++;

        // Check if the current step is complete
        if (_sample_index_within_step >= _samples_per_step) {
            // Reset the sample index within the step
            _sample_index_within_step = 0;
            // Increment the output frequency of sweep driver
            _current_freq += _freq_step;

            // If the incremented frequency is larger than the maximum frequency
            // imposed by the user, then reset the output frequency to _freq0
            if (_current_freq > _max_freq) {
                _current_freq = _freq0;
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

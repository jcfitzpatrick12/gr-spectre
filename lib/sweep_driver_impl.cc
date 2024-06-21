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

using input_type = float;
using output_type = float;

sweep_driver::sptr sweep_driver::make(
    float min_freq, float max_freq, float freq_step, int samp_rate, int samples_per_step)
{
    return gnuradio::make_block_sptr<sweep_driver_impl>(
        min_freq, max_freq, freq_step, samp_rate, samples_per_step);
}


/*
 * The private constructor
 */
sweep_driver_impl::sweep_driver_impl(
    float min_freq, float max_freq, float freq_step, int samp_rate, int samples_per_step)
    : gr::sync_block("sweep_driver",
                     gr::io_signature::make(
                         0, 0, 0),
                     gr::io_signature::make(1, 1, sizeof(output_type))),
    _min_freq(min_freq),
    _max_freq(max_freq),
    _freq_step(freq_step),
    _samp_rate(samp_rate),
    _samples_per_step(samples_per_step),
    _sample_counter(0),
    _freq0(compute_freq0()),
    _current_freq(_freq0)
{   
    // declare message port
    message_port_register_out(pmt::mp("freq"));
    // and publish the (initial) current tuning frequency
    publish_current_freq();
}

/*
 * Our virtual destructor.
 */
sweep_driver_impl::~sweep_driver_impl() {}


float sweep_driver_impl::compute_freq0() {
    return (_min_freq + _samp_rate/2);
}

void sweep_driver_impl::publish_current_freq() {
    pmt::pmt_t key = pmt::intern("freq");  // Key
    pmt::pmt_t value = pmt::from_double(_current_freq);  // Value as double
    pmt::pmt_t msg = pmt::cons(key, value);  // Create pair
    message_port_pub(pmt::mp("freq"), msg);  // Publish the message
}

int sweep_driver_impl::work(int noutput_items,
                            gr_vector_const_void_star& input_items,
                            gr_vector_void_star& output_items)
{
    // cast the (only) output port as a pointer to a float
    float* optr = static_cast<float*>(output_items[0]);

    // let's say the noutput_items <= _samples_per_step
    int k = 0;
    while (k < noutput_items) {
        // if the sample counter has surpassed the max samples per step, reset the counter and increment the frequency
        if (_sample_counter >= _samples_per_step) {
            _sample_counter = 0;
            _current_freq += _freq_step;
            // if the incremented frequency is larger than the max frequency, reset the sweep
            if (_current_freq > _max_freq) {
                _current_freq = _freq0;
            }
            // and publish the current tuning frequency
            publish_current_freq();
        }
        else {
            // output the current frequency
            optr[k] = _current_freq;
            // increment the sample frequency
            _sample_counter += 1;
            // increment the global sample counter
            k += 1;
        }
    }
    return noutput_items;
    }

} /* namespace spectre */
} /* namespace gr */

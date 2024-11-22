/* -*- c++ -*- */
/*
 * Copyright 2024 Jimmy Fitzpatrick.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SPECTRE_SWEEP_DRIVER_IMPL_H
#define INCLUDED_SPECTRE_SWEEP_DRIVER_IMPL_H

#include <gnuradio/spectre/sweep_driver.h>

namespace gr {
namespace spectre {


const pmt::pmt_t OUTPUT_PORT { pmt::string_to_symbol("freq") };


class sweep_driver_impl : public sweep_driver
{
private:
    // User-configured member variables.
    // The lower frequency bound for the frequency sweep [Hz].
    const float _min_freq; 
    // The upper frequency bound for the frequency sweep [Hz].
    const float _max_freq;
    // Increment the center frequency by _freq_step [Hz] after collecting _samples_per_step samples.
    const float _freq_step;
    // The physical sample rate of the input stream.
    const int _samp_rate;
    // The number of samples which are collected before the center frequency is incremented. 
    const int _samples_per_step; 
    // Specify the intended destination message port name. < DOES THIS HAVE ANY EFFECT? >
    pmt::pmt_t _receiver_port_name;
    
    // Internally managed member variables.
    // The initial center frequency.
    const float _initial_freq; 
    // 1-index the sample within each step.
    int _sample_count; 
    // The current active center frequency.
    float _current_freq;

public:

    // Constructor.
    sweep_driver_impl(float min_freq,
                      float max_freq,
                      float freq_step,
                      int samp_rate,
                      int samples_per_step,
                      std::string receiver_port_name);
                    
    // Destructor.
    ~sweep_driver_impl();

    // Publish a message with the current active center frequency.
    void publish_current_freq();

    // Compute the initial center freq required to satisfy the user-configured minimum frequency.
    float compute_initial_freq();

    // Where all the action really happens.
    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items);
};

} // namespace spectre
} // namespace gr

#endif /* INCLUDED_SPECTRE_SWEEP_DRIVER_IMPL_H */

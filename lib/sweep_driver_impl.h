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


const pmt::pmt_t OUTPUT_PORT { pmt::string_to_symbol("retune_command") };


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
    // The name of the command to retune the center frequency.
    pmt::pmt_t _retune_cmd_name;
    
    // Internally managed member variables.
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
                      std::string retune_cmd_name);
                    
    // Destructor.
    ~sweep_driver_impl();

    // Publish a message with the current active center frequency.
    void publish_retune_command();

    // Where all the action really happens.
    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items);
};

} // namespace spectre
} // namespace gr

#endif /* INCLUDED_SPECTRE_SWEEP_DRIVER_IMPL_H */

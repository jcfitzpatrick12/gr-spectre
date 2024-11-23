/* -*- c++ -*- */
/*
 * Copyright 2024 Jimmy Fitzpatrick.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SPECTRE_TAGGED_STAIRCASE_IMPL_H
#define INCLUDED_SPECTRE_TAGGED_STAIRCASE_IMPL_H

#include <gnuradio/spectre/tagged_staircase.h>

namespace gr {
namespace spectre {

const pmt::pmt_t TAG_KEY { pmt::string_to_symbol("rx_freq") };

class tagged_staircase_impl : public tagged_staircase
{
private:
    // User-configured member variables.
    // The minimum step length.
    const int _min_samples_per_step;
    // The maximum step length.
    const int _max_samples_per_step;
    // Increment the modelled center frequency by _freq_step [Hz] every new step.
    const float _freq_step;
    // Incremement the length of each step by this many samples each new step.
    const int _step_increment; 
    // The artificially assigned sample rate.
    const int _samp_rate; 

    // Internally managed member variables.
    // 1-index the sample within each step.
    int _sample_count;
    // 1-index the step.
    int _step_count;
    // The number of samples in the current step.
    int _current_samples_per_step;
    // The artificially assigned initial center frequency.
    float _initial_freq;
    // The current active center frequency.
    float _current_freq; 

public:
    // Constructor.
    tagged_staircase_impl(int min_samples_per_step,
                          int max_samples_per_step,
                          float freq_step,
                          int step_increment,
                          int samp_rate);

    // Destructor.
    ~tagged_staircase_impl();

     // Tag a sample in the output buffer, with index relative to the current call of the work function.
    void tag_step(int rel_sample_index);

    // Compute the initial center frequency which is artificially assigned to the first step.
    float compute_initial_freq();

    // Where all the action really happens.
    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items);
};

} // namespace spectre
} // namespace gr

#endif /* INCLUDED_SPECTRE_TAGGED_STAIRCASE_IMPL_H */

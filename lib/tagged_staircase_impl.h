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

class tagged_staircase_impl : public tagged_staircase
{
private:
    const int _min_samples_per_step; // User input. The minimum step length (with respect to number of samples).
    const int _max_samples_per_step; // User input. The maximum step length (with respect to number of samples).
    const int _step_increment; // User input. The "height" of each step, in terms of the block output.
    const int _samp_rate; // User input. The (imposed) sample rate.
    int _sample_index_within_step; // Keeps track of the index within a step (0-indexed)
    int _step_index; // Keeps track of the index of the step (0-indexed)
    int _current_samples_per_step; // Keeps track of the current step length (i.e. the number of samples belonging to the current step)
    float _samp_rate_as_float;  // Member variable to store the sample rate type cast as float
    float _min_modelled_frequency; // The artificially assigned center frequency assigned to the first step.
    float _current_modelled_frequency; // The artifically assigned center frequency for the current step.

public:
    tagged_staircase_impl(int min_samples_per_step,
                          int max_samples_per_step,
                          int step_increment,
                          int samp_rate);
    ~tagged_staircase_impl();
     // member function which tags the ith sample in the output buffer
    void tag_step(int i);
    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items);
};

} // namespace spectre
} // namespace gr

#endif /* INCLUDED_SPECTRE_TAGGED_STAIRCASE_IMPL_H */

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
    int _min_samples_per_step; /**< user input */
    int _max_samples_per_step; /**< user input */
    int _step_increment; /**< user input */
    int _samp_rate; /**< user input */
    int _sample_counter; /**< keeps track of the index within a step */
    int _step_counter; /**< keeps track of the index of the step */
    int _current_samples_per_step; /**< keeps track of the current step length */
    float _samp_rate_as_float;  /**< Type cast sample rate as float*/
    float _min_modelled_frequency; /**< The artificially assigned center frequency assigned to the first step*/
    float _current_modelled_frequency; /**< The artifically assigned center frequency for the current step*/

public:
    tagged_staircase_impl(int min_samples_per_step,
                          int max_samples_per_step,
                          int step_increment,
                          int samp_rate);
    ~tagged_staircase_impl();
     // member function which tags the ith sample in the output buffer
    void tag_step(int i);
    // Where all the action really happens
    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items);
};

} // namespace spectre
} // namespace gr

#endif /* INCLUDED_SPECTRE_TAGGED_STAIRCASE_IMPL_H */

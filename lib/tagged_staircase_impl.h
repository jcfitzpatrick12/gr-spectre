/*
 * Copyright 2024-2026 Jimmy Fitzpatrick.
 * This file is part of SPECTRE
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SPECTRE_TAGGED_STAIRCASE_IMPL_H
#define INCLUDED_SPECTRE_TAGGED_STAIRCASE_IMPL_H

#include <gnuradio/spectre/tagged_staircase.h>

namespace gr {
namespace spectre {

const pmt::pmt_t TAG_KEY = pmt::string_to_symbol("rx_freq");

class tagged_staircase_impl : public tagged_staircase
{
public:
    tagged_staircase_impl(int min_samples_per_step,
                          int max_samples_per_step,
                          float hop_freq,
                          int step_increment,
                          float sample_rate);

    ~tagged_staircase_impl();

    void tag_step(int rel_sample_index);

    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items);

private:
    const int d_min_samples_per_step;
    const int d_max_samples_per_step;
    const float d_hop_freq;
    const int d_step_increment;
    const float d_initial_freq;

    int d_nstep;
    int d_nsamples;
    int d_nsamples_per_step;
    float d_active_freq;
};

} // namespace spectre
} // namespace gr

#endif /* INCLUDED_SPECTRE_TAGGED_STAIRCASE_IMPL_H */

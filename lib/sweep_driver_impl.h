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

class sweep_driver_impl : public sweep_driver
{
private:
    // Nothing to declare in this block.

public:
    sweep_driver_impl(float min_freq,
                      float max_freq,
                      float freq_step,
                      int samp_rate,
                      int samples_per_step);
    ~sweep_driver_impl();

    // Where all the action really happens
    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items);
};

} // namespace spectre
} // namespace gr

#endif /* INCLUDED_SPECTRE_SWEEP_DRIVER_IMPL_H */

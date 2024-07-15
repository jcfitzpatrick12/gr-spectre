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
    const float _min_freq; /**< User input. The lower bound for the frequency sweep. */
    const float _max_freq; /**< User input. The upper bound for the frequency sweep. */
    const float _freq_step; /**< User input. Step _freq_step [Hz] after collecting _samples_per_step samples. */
    const int _samp_rate; /**< User input. The (imposed) sample rate. */
    const int _samples_per_step; /**< User input. The number of samples per step. */
    const float _freq0; /**< The frequency at which we start the steps, based on min_freq. */
    int _sample_index_within_step; /**< 0-indexes the sample within the step. */
    float _current_freq; /**< Stores the current active frequency. */

public:
    sweep_driver_impl(float min_freq,
                      float max_freq,
                      float freq_step,
                      int samp_rate,
                      int samples_per_step);
    ~sweep_driver_impl();
    void publish_current_freq();
    float compute_freq0(); /**< Compute the initial frequency based on constructor inputs. */
    // Where all the action really happens
    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items);
};

} // namespace spectre
} // namespace gr

#endif /* INCLUDED_SPECTRE_SWEEP_DRIVER_IMPL_H */

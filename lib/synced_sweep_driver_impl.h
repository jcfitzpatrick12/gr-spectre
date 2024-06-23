/* -*- c++ -*- */
/*
 * Copyright 2024 Jimmy Fitzpatrick.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SPECTRE_SYNCED_SWEEP_DRIVER_IMPL_H
#define INCLUDED_SPECTRE_SYNCED_SWEEP_DRIVER_IMPL_H

#include <gnuradio/spectre/synced_sweep_driver.h>

namespace gr {
namespace spectre {

class synced_sweep_driver_impl : public synced_sweep_driver
{
private:
    float _min_freq; /**< The lower bound for the frequency sweep. */
    float _max_freq; /**< The upper bound for the frequency sweep. */
    float _freq_step; /**< Step _freq_step [Hz] after collecting _samples_per_step samples. */
    int _samp_rate; /**< The (imposed) sample rate.*/
    int _samples_per_step; /**< The number of samples to collect at each step.*/
    int _sample_counter; /**< To keep track of the sample count at each step.*/
    float _freq0; /**< The frequency at which we start the steps, based on min_freq. */
    float _current_freq; /**< Stores the current active frequency. */

public:
    synced_sweep_driver_impl(float min_freq,
                             float max_freq,
                             float freq_step,
                             int samp_rate,
                             int samples_per_step);
    ~synced_sweep_driver_impl();
    void publish_current_freq();
    float compute_freq0(); /**< Compute the initial frequency based on constructor inputs. */
    // Where all the action really happens
    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items);
};

} // namespace spectre
} // namespace gr

#endif /* INCLUDED_SPECTRE_SYNCED_SWEEP_DRIVER_IMPL_H */

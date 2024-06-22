/* -*- c++ -*- */
/*
 * Copyright 2024 Jimmy Fitzpatrick.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SPECTRE_SYNCED_SWEEP_DRIVER_H
#define INCLUDED_SPECTRE_SYNCED_SWEEP_DRIVER_H

#include <gnuradio/spectre/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace spectre {

/*!
 * \brief <+description of block+>
 * \ingroup spectre
 *
 */
class SPECTRE_API synced_sweep_driver : virtual public gr::sync_block
{
public:
    typedef std::shared_ptr<synced_sweep_driver> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of spectre::synced_sweep_driver.
     *
     * To avoid accidental use of raw pointers, spectre::synced_sweep_driver's
     * constructor is in a private implementation
     * class. spectre::synced_sweep_driver::make is the public interface for
     * creating new instances.
     */
    static sptr make(float min_freq = 40000000,
                     float max_freq = 80000000,
                     float freq_step = 5000000,
                     int samp_rate = 4000000,
                     int samples_per_step = 4000000);
};

} // namespace spectre
} // namespace gr

#endif /* INCLUDED_SPECTRE_SYNCED_SWEEP_DRIVER_H */

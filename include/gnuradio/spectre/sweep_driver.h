/* -*- c++ -*- */
/*
 * Copyright 2024 Jimmy Fitzpatrick.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SPECTRE_SWEEP_DRIVER_H
#define INCLUDED_SPECTRE_SWEEP_DRIVER_H

#include <gnuradio/spectre/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace spectre {

/*!
 * \brief <+description of block+>
 * \ingroup spectre
 *
 */
class SPECTRE_API sweep_driver : virtual public gr::sync_block
{
public:
    typedef std::shared_ptr<sweep_driver> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of spectre::sweep_driver.
     *
     * To avoid accidental use of raw pointers, spectre::sweep_driver's
     * constructor is in a private implementation
     * class. spectre::sweep_driver::make is the public interface for
     * creating new instances.
     */
    static sptr make(float min_freq = 10000000,
                     float max_freq = 100000000,
                     float freq_step = 300000,
                     int samp_rate = 300000,
                     int samples_per_step = 512);
};

} // namespace spectre
} // namespace gr

#endif /* INCLUDED_SPECTRE_SWEEP_DRIVER_H */

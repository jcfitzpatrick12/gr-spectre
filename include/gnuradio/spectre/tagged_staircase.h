/* -*- c++ -*- */
/*
 * Copyright 2024 Jimmy Fitzpatrick.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SPECTRE_TAGGED_STAIRCASE_H
#define INCLUDED_SPECTRE_TAGGED_STAIRCASE_H

#include <gnuradio/spectre/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace spectre {

/*!
 * \brief <+description of block+>
 * \ingroup spectre
 *
 */
class SPECTRE_API tagged_staircase : virtual public gr::sync_block
{
public:
    typedef std::shared_ptr<tagged_staircase> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of spectre::tagged_staircase.
     *
     * To avoid accidental use of raw pointers, spectre::tagged_staircase's
     * constructor is in a private implementation
     * class. spectre::tagged_staircase::make is the public interface for
     * creating new instances.
     */
    static sptr make(int min_samples_per_step = 4000,
                     int max_samples_per_step = 5000,
                     int step_increment = 200,
                     int samp_rate = 32000);
};

} // namespace spectre
} // namespace gr

#endif /* INCLUDED_SPECTRE_TAGGED_STAIRCASE_H */

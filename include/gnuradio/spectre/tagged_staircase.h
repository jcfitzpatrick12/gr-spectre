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
 * \brief Models I/Q samples produced by a receiver whose center frequency is swept over a
 * range of frequencies. \ingroup spectre \details Produces complex samples where the real
 * component is incremented by a unit after some number of samples and the imaginary
 * component is always zero. Consecutive samples with identical values form a step,
 * modeling samples captured at the same center frequency. The first sample of each step
 * is tagged with the corresponding center frequency. Step lengths increase by a fixed
 * amount compared to the previous, until it would surpass a maximum length (then it's
 * reset).
 *
 * The motivation is to validate spectrograms produced by Spectre by enabling direct
 * comparison with analytical results. The constant value within each step allows explicit
 * evaluation of the DFT, while the variable step lengths reflect the behavior of a
 * receiver periodically retuned using message passing in GNU Radio.
 */
class SPECTRE_API tagged_staircase : virtual public gr::sync_block
{
public:
    typedef std::shared_ptr<tagged_staircase> sptr;

    /*!
     * \brief Make a tagged staircase.
     * \param min_samples_per_step The length of the shortest step in the staircase
     * \param max_samples_per_step The maximum allowed length (inclusive) of a step in the
     * staircase
     * \param hop_freq The amount to increment the modelled center frequency at each new
     * step \param step_increment Each new step increases in length by this amount
     * \param sample_rate The modelled sample rate of the output stream. According to this
     * value, the initial modelled center frequency is set such that on performing an FFT,
     * the edge of the spectrum will be at 0Hz
     */
    static sptr make(int min_samples_per_step = 4000,
                     int max_samples_per_step = 5000,
                     float hop_freq = 32000,
                     int step_increment = 200,
                     float sample_rate = 32000);
};

} // namespace spectre
} // namespace gr

#endif /* INCLUDED_SPECTRE_TAGGED_STAIRCASE_H */

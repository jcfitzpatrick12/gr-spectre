/*
 * Copyright 2024-2026 Jimmy Fitzpatrick.
 * This file is part of SPECTRE
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SPECTRE_FREQUENCY_SWEEPER_H
#define INCLUDED_SPECTRE_FREQUENCY_SWEEPER_H

#include <gnuradio/spectre/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace spectre {

/*!
 * \brief Sweep compatible receiver blocks over a range of frequencies in
 * fixed increments. \ingroup spectre
 *
 * \details Periodically retunes compatible receiver blocks over a range of frequencies in
 * fixed increments using message passing. Dwell time is measured through sample counting.
 */
class SPECTRE_API frequency_sweeper : virtual public gr::sync_block
{
public:
    typedef std::shared_ptr<frequency_sweeper> sptr;

    /*!
     * \brief Make a frequency sweeper.
     * \param min_freq The minimum center frequency in the sweep (inclusive)
     * \param max_freq The maximum center frequency in the sweep (inclusive)
     * \param hop_freq The amount to increment the center frequency at each step
     * in the sweep.
     * \param dwell_time The amount of time to spend at each frequency in the sweep.
     * \param sample_rate The sample rate of the input stream.
     * \param retune_cmd_name The name of the retune command (please consult the
     * receiver block implementation).
     * \param input_type The data type of each sample in the input stream.
     */
    static sptr make(float min_freq = 90e6,
                     float max_freq = 1106,
                     float hop_freq = 2e6,
                     float dwell_time = 200e-3,
                     float sample_rate = 2e6,
                     const std::string& retune_cmd_name = "freq",
                     const std::string& input_type = "fc32");
};

} // namespace spectre
} // namespace gr

#endif /* INCLUDED_SPECTRE_FREQUENCY_SWEEPER_H */

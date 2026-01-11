/*
 * Copyright 2024-2026 Jimmy Fitzpatrick.
 * This file is part of SPECTRE
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SPECTRE_FREQUENCY_SWEEPER_IMPL_H
#define INCLUDED_SPECTRE_FREQUENCY_SWEEPER_IMPL_H

#include <gnuradio/spectre/frequency_sweeper.h>

namespace gr {
namespace spectre {

const pmt::pmt_t OUTPUT_PORT{ pmt::string_to_symbol("retune_command") };

class frequency_sweeper_impl : public frequency_sweeper
{
public:
    frequency_sweeper_impl(float min_freq,
                           float max_freq,
                           float hop_freq,
                           float dwell_time,
                           float sample_rate,
                           const std::string& retune_cmd_name,
                           const std::string& input_type);
    ~frequency_sweeper_impl();

    void publish_retune_command();
    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items);

private:
    const float d_min_freq;
    const float d_max_freq;
    const float d_hop_freq;
    const float d_nsamples_per_step;
    pmt::pmt_t d_retune_cmd_name;
    int d_nsamples;
    float d_active_freq;
};

} // namespace spectre
} // namespace gr

#endif /* INCLUDED_SPECTRE_FREQUENCY_SWEEPER_IMPL_H */

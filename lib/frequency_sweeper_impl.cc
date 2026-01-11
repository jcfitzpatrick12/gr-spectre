/*
 * Copyright 2024-2026 Jimmy Fitzpatrick.
 * This file is part of SPECTRE
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "frequency_sweeper_impl.h"
#include "utils.h"
#include <gnuradio/io_signature.h>

namespace {
int get_num_samples_per_step(float dwell_time, float sample_rate)
{
    // Naturally, we can't have a non-integral number of samples per step,
    // so we floor to ensure that the elapsed time per step doesn't surpass the
    // user-configured dwell time.
    return std::floor(dwell_time * sample_rate);
}
} // namespace


namespace gr {
namespace spectre {

frequency_sweeper::sptr frequency_sweeper::make(float min_freq,
                                                float max_freq,
                                                float hop_freq,
                                                float dwell_time,
                                                float sample_rate,
                                                const std::string& retune_cmd_name,
                                                const std::string& input_type)
{
    return gnuradio::make_block_sptr<frequency_sweeper_impl>(min_freq,
                                                             max_freq,
                                                             hop_freq,
                                                             dwell_time,
                                                             sample_rate,
                                                             retune_cmd_name,
                                                             input_type);
}


frequency_sweeper_impl::frequency_sweeper_impl(float min_freq,
                                               float max_freq,
                                               float hop_freq,
                                               float dwell_time,
                                               float sample_rate,
                                               const std::string& retune_cmd_name,
                                               const std::string& input_type)
    : gr::sync_block("frequency_sweeper",
                     gr::io_signature::make(1, 1, get_sizeof_stream_item(input_type)),
                     gr::io_signature::make(0, 0, 0)),
      d_min_freq(min_freq),
      d_max_freq(max_freq),
      d_hop_freq(hop_freq),
      d_nsamples_per_step(get_num_samples_per_step(dwell_time, sample_rate)),
      d_retune_cmd_name(pmt::string_to_symbol(retune_cmd_name)),
      d_nsamples(0),
      d_active_freq(min_freq)
{
    message_port_register_out(OUTPUT_PORT);
}

frequency_sweeper_impl::~frequency_sweeper_impl() {}

void frequency_sweeper_impl::publish_retune_command()
{
    pmt::pmt_t retune_command = pmt::make_dict();
    retune_command =
        pmt::dict_add(retune_command, d_retune_cmd_name, pmt::from_float(d_active_freq));
    message_port_pub(OUTPUT_PORT, retune_command);
}

int frequency_sweeper_impl::work(int noutput_items,
                                 gr_vector_const_void_star& input_items,
                                 gr_vector_void_star& output_items)
{
    for (int n = 0; n < noutput_items; n++) {
        // Measure elapsed time by counting samples.
        d_nsamples++;
        if (d_nsamples == d_nsamples_per_step) {
            // If we've reached the dwell time, increment the center frequency, resetting
            // it if it's out of range
            d_active_freq += d_hop_freq;
            if (d_active_freq > d_max_freq) {
                d_active_freq = d_min_freq;
            }

            // Issue the command to retune the receiver.
            publish_retune_command();

            // At the next call to work, start recounting samples afresh.
            d_nsamples = 0;
        }
    }
    return noutput_items;
}

} /* namespace spectre */
} /* namespace gr */

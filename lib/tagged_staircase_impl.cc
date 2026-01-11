/* -*- c++ -*- */
/*
 * Copyright 2024 Jimmy Fitzpatrick.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "tagged_staircase_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/tagged_stream_block.h>

namespace {

static constexpr int OUTPUT_PORT = 0;

float compute_initial_freq(const float sample_rate)
{
    // Compute the initial center frequency such that
    // on performing an FFT, the edge of the spectrum will be at 0Hz.
    return (sample_rate / 2);
}

} // namespace


namespace gr {
namespace spectre {


using output_type = gr_complex;


tagged_staircase::sptr tagged_staircase::make(int min_samples_per_step,
                                              int max_samples_per_step,
                                              float hop_freq,
                                              int step_increment,
                                              float sample_rate)
{
    return gnuradio::make_block_sptr<tagged_staircase_impl>(min_samples_per_step,
                                                            max_samples_per_step,
                                                            hop_freq,
                                                            step_increment,
                                                            sample_rate);
}


tagged_staircase_impl::tagged_staircase_impl(int min_samples_per_step,
                                             int max_samples_per_step,
                                             float hop_freq,
                                             int step_increment,
                                             float sample_rate)
    : gr::sync_block("tagged_staircase",
                     gr::io_signature::make(0, 0, 0),
                     gr::io_signature::make(1, 1, sizeof(output_type))),
      d_min_samples_per_step(min_samples_per_step),
      d_max_samples_per_step(max_samples_per_step),
      d_hop_freq(hop_freq),
      d_step_increment(step_increment),
      d_initial_freq(compute_initial_freq(sample_rate)),
      d_nstep(0),
      d_nsamples(0),
      d_nsamples_per_step(min_samples_per_step),
      d_active_freq(d_initial_freq)
{
}


tagged_staircase_impl::~tagged_staircase_impl() {}

void tagged_staircase_impl::tag_step(int rel_sample_index)
{
    // Tag the first sample in a new step using the sample index relative to the current
    // work call.
    const uint64_t absolute_offset = nitems_written(0) + rel_sample_index;
    const pmt::pmt_t value = pmt::from_float(d_active_freq);
    const pmt::pmt_t srcid = pmt::string_to_symbol(alias());
    add_item_tag(OUTPUT_PORT, absolute_offset, TAG_KEY, value, srcid);
}


int tagged_staircase_impl::work(int noutput_items,
                                gr_vector_const_void_star& input_items,
                                gr_vector_void_star& output_items)
{

    output_type* out = static_cast<output_type*>(output_items[0]);

    for (int n = 0; n < noutput_items; n++) {
        if (d_nsamples == 0) {
            // Tag the first sample of each step.
            tag_step(n);
        }
        // Output the current step count (1-indexed) to the real component of the output,
        // keeping a record of how many samples we've produced in the current step.
        out[n] = output_type(static_cast<float>(d_nstep + 1), 0.0f);
        d_nsamples++;

        if (d_nsamples == d_nsamples_per_step) {
            // Start a new step, resetting the sample counter and incrementing the
            // modelled frequency.
            d_nstep++;
            d_nsamples = 0;
            d_active_freq += d_hop_freq;

            // The new step grows in length by a fixed number of samples.
            d_nsamples_per_step += d_step_increment;

            if (d_nsamples_per_step > d_max_samples_per_step) {
                // If we exceed the maximum size of a step, reset to the shortest.
                d_nsamples_per_step = d_min_samples_per_step;
                d_nstep = 0;
                d_active_freq = d_initial_freq;
            }
        }
    }

    return noutput_items;
}

} /* namespace spectre */
} /* namespace gr */

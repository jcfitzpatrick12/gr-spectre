/* -*- c++ -*- */
/*
 * Copyright 2024 Jimmy Fitzpatrick.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "tagged_staircase_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/tagged_stream_block.h>


namespace gr {
namespace spectre {


using output_type = gr_complex;


tagged_staircase::sptr tagged_staircase::make(int min_samples_per_step,
                                              int max_samples_per_step,
                                              float freq_step,
                                              int step_increment,
                                              int samp_rate)
{
    return gnuradio::make_block_sptr<tagged_staircase_impl>(
        min_samples_per_step, max_samples_per_step, freq_step, step_increment, samp_rate);
}


tagged_staircase_impl::tagged_staircase_impl(int min_samples_per_step,
                                             int max_samples_per_step,
                                             float freq_step,
                                             int step_increment,
                                             int samp_rate)
    : gr::sync_block("tagged_staircase",
                     gr::io_signature::make(0, 0, 0),
                     gr::io_signature::make(
                         1 /* min outputs */, 1 /*max outputs */, sizeof(output_type))),
      _min_samples_per_step(min_samples_per_step),
      _max_samples_per_step(max_samples_per_step),
      _freq_step(freq_step),
      _step_increment(step_increment),
      _samp_rate(samp_rate),
      _sample_count(0),
      _step_count(0),
      _current_samples_per_step(_min_samples_per_step),
      _initial_freq(compute_initial_freq()),
      _current_freq(_initial_freq)
{
}


tagged_staircase_impl::~tagged_staircase_impl() {}


float tagged_staircase_impl::compute_initial_freq() 
{
    // Compute the initial center frequency such that
    // on performing an FFT with the defined sampling rate, 
    // the edge of the spectrum will be at 0Hz.
    return (static_cast<float>(_samp_rate) / 2);
}


void tagged_staircase_impl::tag_step(int rel_sample_index) 
{
    // nitems_written is w.r.t. the state at the start of the call to work.
    // So, to tag the appropriate sample in the output buffer, we append
    // the relative sample index, to the number of items written as of 
    // the start of the call to the work function.
    const uint64_t absolute_offset = nitems_written(0) + rel_sample_index;
    const pmt::pmt_t value = pmt::from_float(_current_freq);
    const pmt::pmt_t srcid = pmt::string_to_symbol(alias());
    add_item_tag(0, absolute_offset, TAG_KEY, value, srcid);
}


int tagged_staircase_impl::work(int noutput_items,
                                gr_vector_const_void_star& input_items,
                                gr_vector_void_star& output_items)
{   
    // Type cast the pointer to the output buffer.
    output_type* optr = static_cast<output_type*>(output_items[0]);


    // Tag the absolute first sample of the stream.
    // This will only be true only at the first call of the work function.
    // (since only at the first call, will no samples have been written
    // to the output buffer).
    if (nitems_written(0) == 0)
    {
        // Tag the very first sample in the stream.
        tag_step(0);
        // Increment the step counter (to indicate we are starting the first step)
        _step_count++;
    }

    for (int i = 0; i < noutput_items; i++) 
    {
        // Increment the sample count
        _sample_count++;

        // Output the current step count to the real component of the complex output.
        optr[i] = output_type(static_cast<float>(_step_count), 0.0f);

        // Check if the current step is complete
        if (_sample_count == _current_samples_per_step) 
        {
            // Reset the sample count.
            _sample_count = 0;
            // Increment the step counter
            _step_count++;
            // Increment the number of samples per step.
            _current_samples_per_step += _step_increment;
            // Increment the modelled center frequency.
            _current_freq += _samp_rate;
            // Reset if we exceed the maximum step size
            if (_current_samples_per_step > _max_samples_per_step) 
            {
                // Reset the number of samples per step.
                _current_samples_per_step = _min_samples_per_step;
                // Reset the step count (back to step number one)
                _step_count = 1;
                // Reset the modelled center frequency
                _current_freq = _initial_freq;
            }

            // Tag the first sample of the new step.
            tag_step(i + 1);

        }
    }

    // Return the number of output items produced
    return noutput_items;
}

} /* namespace spectre */
} /* namespace gr */

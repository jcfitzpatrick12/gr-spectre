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
                                              int step_increment,
                                              int samp_rate)
{
    return gnuradio::make_block_sptr<tagged_staircase_impl>(
        min_samples_per_step, max_samples_per_step, step_increment, samp_rate);
}


/*
 * The private constructor
 */
tagged_staircase_impl::tagged_staircase_impl(int min_samples_per_step,
                                             int max_samples_per_step,
                                             int step_increment,
                                             int samp_rate)
    : gr::sync_block("tagged_staircase",
                     gr::io_signature::make(0, 0, 0),
                     gr::io_signature::make(
                         1 /* min outputs */, 1 /*max outputs */, sizeof(output_type))),
      _min_samples_per_step(min_samples_per_step),
      _max_samples_per_step(max_samples_per_step),
      _step_increment(step_increment),
      _samp_rate(samp_rate),
      _sample_counter(0),
      _step_counter(0),
      _current_samples_per_step(_min_samples_per_step),
      _samp_rate_as_float(static_cast<float>(samp_rate)),
      _min_modelled_frequency(static_cast<float>(samp_rate) / 2.0f),
      _current_modelled_frequency(static_cast<float>(samp_rate) / 2.0f)
{
    tag_step(0);
}

/*
 * Our virtual destructor.
 */
tagged_staircase_impl::~tagged_staircase_impl() {}


void tagged_staircase_impl::tag_step(int sample_index) {
    // Add a tag to the first sample of the new step
    const uint64_t absolute_offset = nitems_written(0) + sample_index;
    const pmt::pmt_t key = pmt::intern("freq");
    const pmt::pmt_t value = pmt::from_float(_current_modelled_frequency);
    const pmt::pmt_t srcid = pmt::string_to_symbol(alias());
    add_item_tag(0, absolute_offset, key, value, srcid);
}


int tagged_staircase_impl::work(int noutput_items,
                                gr_vector_const_void_star& input_items,
                                gr_vector_void_star& output_items)
{
    // Type cast the output buffer to point to output_type
    output_type* optr = static_cast<output_type*>(output_items[0]);

    // Process each output item
    for (int i = 0; i < noutput_items; i++) {
        // Output the current step (1-based) index
        optr[i] = output_type(static_cast<float>(_step_counter + 1), 0.0f);

        // Increment the sample counter
        _sample_counter++;

        // Check if the current step is complete
        if (_sample_counter >= _current_samples_per_step) {
            // tag the first sample of the new step
            tag_step(i);
            // Move to the next step, reset the sample counter
            _sample_counter = 0;
            // increment the step counter
            _step_counter++;
            // increment the samples per step
            _current_samples_per_step += _step_increment;
            // and increment the modelled frequnecy
            _current_modelled_frequency += _samp_rate_as_float;

            // reset if we exceed the maximum step size
            if (_current_samples_per_step > _max_samples_per_step) {
                _current_samples_per_step = _min_samples_per_step;
                _step_counter = 0;
                _current_modelled_frequency = _min_modelled_frequency;
            }
        }
    }

    // Return the number of output items produced
    return noutput_items;
}

} /* namespace spectre */
} /* namespace gr */

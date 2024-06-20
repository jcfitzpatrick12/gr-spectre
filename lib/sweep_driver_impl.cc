/* -*- c++ -*- */
/*
 * Copyright 2024 Jimmy Fitzpatrick.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "sweep_driver_impl.h"
#include <gnuradio/io_signature.h>

namespace gr {
namespace spectre {

#pragma message("set the following appropriately and remove this warning")
using input_type = float;
sweep_driver::sptr sweep_driver::make(
    float min_freq, float max_freq, float freq_step, int samp_rate, int samples_per_step)
{
    return gnuradio::make_block_sptr<sweep_driver_impl>(
        min_freq, max_freq, freq_step, samp_rate, samples_per_step);
}


/*
 * The private constructor
 */
sweep_driver_impl::sweep_driver_impl(
    float min_freq, float max_freq, float freq_step, int samp_rate, int samples_per_step)
    : gr::sync_block("sweep_driver",
                     gr::io_signature::make(
                         1 /* min inputs */, 1 /* max inputs */, sizeof(input_type)),
                     gr::io_signature::make(0, 0, 0))
{
}

/*
 * Our virtual destructor.
 */
sweep_driver_impl::~sweep_driver_impl() {}

int sweep_driver_impl::work(int noutput_items,
                            gr_vector_const_void_star& input_items,
                            gr_vector_void_star& output_items)
{
    auto in = static_cast<const input_type*>(input_items[0]);

#pragma message("Implement the signal processing in your block and remove this warning")
    // Do <+signal processing+>

    // Tell runtime system how many output items we produced.
    return noutput_items;
}

} /* namespace spectre */
} /* namespace gr */

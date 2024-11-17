/* -*- c++ -*- */
/*
 * Copyright 2024 Jimmy Fitzpatrick.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SPECTRE_BATCHED_FILE_SINK_H
#define INCLUDED_SPECTRE_BATCHED_FILE_SINK_H

#include <gnuradio/spectre/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace spectre {

/*!
 * \brief <+description of block+>
 * \ingroup sandbox
 *
 */
class SPECTRE_API batched_file_sink : virtual public gr::sync_block
{
public:
    typedef std::shared_ptr<batched_file_sink> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of sandbox::batched_file_sink.
     *
     * To avoid accidental use of raw pointers, sandbox::batched_file_sink's
     * constructor is in a private implementation
     * class. sandbox::batched_file_sink::make is the public interface for
     * creating new instances.
     */
    static sptr make(const std::string parent_dir_path = "./",
                     const std::string tag = "my-tag",
                     const float batch_size = 3.0,
                     const int samp_rate = 32000,
                     const bool is_sweeping = false,
                     const std::string frequency_tag_key = "freq",
                     const float initial_center_frequency = 0);
};

} // namespace spectre
} // namespace gr

#endif /* INCLUDED_SPECTRE_BATCHED_FILE_SINK_H */

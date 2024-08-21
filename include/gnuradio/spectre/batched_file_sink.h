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
 * \ingroup spectre
 *
 */
class SPECTRE_API batched_file_sink : virtual public gr::sync_block
{
public:
    typedef std::shared_ptr<batched_file_sink> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of spectre::batched_file_sink.
     *
     * To avoid accidental use of raw pointers, spectre::batched_file_sink's
     * constructor is in a private implementation
     * class. spectre::batched_file_sink::make is the public interface for
     * creating new instances.
     */
    static sptr make(std::string parent_dir = "",
                     std::string tag = "",
                     int chunk_size = 60,
                     int samp_rate = 32000,
                     bool sweeping = false,
                     std::string frequency_tag_key = "freq",
                     float initial_active_frequency = 0);
};

} // namespace spectre
} // namespace gr

#endif /* INCLUDED_SPECTRE_BATCHED_FILE_SINK_H */

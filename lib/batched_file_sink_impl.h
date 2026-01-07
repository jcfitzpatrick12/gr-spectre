/*
 * Copyright 2024-2026 Jimmy Fitzpatrick.
 * This file is part of SPECTRE
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SPECTRE_BATCHED_FILE_SINK_IMPL_H
#define INCLUDED_SPECTRE_BATCHED_FILE_SINK_IMPL_H

#include <gnuradio/spectre/batched_file_sink.h>

#include <gnuradio/types.h>
#include <filesystem>
#include <fstream>

namespace gr {
namespace spectre {

struct batch_time {
    std::tm utc_tm;
    int ms;
};

enum buffer_state {
    EMPTY = 0,
    FILLING,
    FULL,
};

class batched_file_sink_impl : public batched_file_sink
{
public:
    batched_file_sink_impl(const std::string& dir,
                           const std::string& tag,
                           const std::string& input_type,
                           const float batch_size,
                           const int sample_rate,
                           const bool group_by_date,
                           const bool is_tagged,
                           const std::string& tag_key,
                           const float initial_tag_value);
    ~batched_file_sink_impl();
    int work(int noutput_items,
             gr_vector_const_void_star& in,
             gr_vector_void_star& out) override;

private:
    const std::string d_dir;
    const std::string d_tag;
    const std::string d_input_type;
    const size_t d_sizeof_stream_item;
    const int d_nsamples_per_batch;
    const bool d_is_tagged;
    const bool d_group_by_date;
    int d_nbuffered_samples;
    std::vector<char> d_buffer;
    std::ofstream d_fdata;
    std::ofstream d_ftags;
    batch_time d_batch_time;
    buffer_state d_buffer_state;


    void init();
    void reset();
    void update_batch_time();
    void open_fstreams();
    void open_fstream(std::ofstream& f, const std::string& extension);
    int fill_buffer(int noutput_items, const unsigned char* in);
    void flush_buffer();
    void close_fstreams();
};

} // namespace spectre
} // namespace gr

#endif
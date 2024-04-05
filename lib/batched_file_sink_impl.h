/* -*- c++ -*- */
/*
 * Copyright 2024 Jimmy Fitzpatrick.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SPECTRE_BATCHED_FILE_SINK_IMPL_H
#define INCLUDED_SPECTRE_BATCHED_FILE_SINK_IMPL_H

#include <gnuradio/spectre/batched_file_sink.h>
#include <iostream>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace gr {
namespace spectre {

class batched_file_sink_impl : public batched_file_sink
{
private:
    std::string _parent_dir; // the directory where the files will be stored
    std::string _tag; // file names will be [start time of collection]_[tag].bin
    std::ofstream _file; // file object to open close and write the data
    int _chunk_size; // each file will contain chunk_size [s] of data
    int _samp_rate; // the sample rate [used to determine time elapsed]
    bool _open_new_file; // bool to keep track of whether to open a new file or not
    double _elapsed_time; // float to keep track of the elapsed time 

public:
    batched_file_sink_impl(std::string parent_dir,
                           std::string tag,
                           int chunk_size,
                           int samp_rate);
    ~batched_file_sink_impl();

    void open_file();
    fs::path make_file_path(const fs::path& parent_dir, const std::string& tag);

    // Where all the action really happens
    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items);
};

} // namespace spectre
} // namespace gr

#endif /* INCLUDED_SPECTRE_BATCHED_FILE_SINK_IMPL_H */
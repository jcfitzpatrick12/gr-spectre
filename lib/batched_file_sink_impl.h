/* -*- c++ -*- */
/*
 * Copyright 2024 Jimmy Fitzpatrick.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SPECTRE_BATCHED_FILE_SINK_IMPL_H
#define INCLUDED_SPECTRE_BATCHED_FILE_SINK_IMPL_H

#include <gnuradio/spectre/batched_file_sink.h>

#include <fstream>
#include <filesystem>
#include <chrono>
#include <ctime>

namespace fs = std::filesystem;

namespace gr {
namespace spectre {

class batched_file_sink_impl : public batched_file_sink
{
private:
    std::string _parent_dir; /**< Defines the absolute path to the parent directory to hold the batched files. */
    std::string _tag; /**< File names are tagged with this string. */
    int _chunk_size; /**< The (temporal) size of each batched file. */
    int _samp_rate; /**< The sample rate used to collect the incoming data stream. */
    std::ofstream _file; /**<  define a ofstream class to handle file opening and read write operations */
    bool _open_new_file; /**< Private attribute to determine whether a new file should be opened at each call of the work function */
    float _elapsed_time; /**< Elapsed time since the file opening for that batch */
    bin_chunk_helper _bch; /**< class which handles the binary chunk file metadata */

public:
    batched_file_sink_impl(
        std::string parent_dir,
        std::string tag,
        int chunk_size,
        int samp_rate
    );

    ~batched_file_sink_impl();

    /**
    * @brief Opens a new batched binary file.
    */
    void open_file(); 
    
    int work(
        int noutput_items,
        gr_vector_const_void_star& input_items,
        gr_vector_void_star& output_ite
    );
};

class bin_chunk_helper
{
private:
    fs::path _parent_path; /**< Defines the absolute path to the parent directory to hold the batched files. */
    std::string _tag; /**< File names are tagged with this string. */
    int32_t _millisecond_correction; /**< A 32-bit integer describing the millisecond correction for the timestamp of the first sample in each batch.*/
    fs::path _absolute_path; /**< The absolute path to the binary chunk file. */

public:
    bin_chunk_helper(
        std::string parent_path,
        std::string tag
    );

    ~bin_chunk_helper();
    void set_attrs(); /**< Effectively a setter for the _millisecond_correction and _absolute_path member variables. */
    fs::path get_absolute_path(); /**< Public getter for the _absolute_path member variable. */
    int32_t get_millisecond_correction(); /**< Public getter for the _millisecond_correction member variable. */
    fs::path get_absolute_parent_path(); /**< Public getter for the parent path of the binary chunk file.*/
};

} // namespace spectre
} // namespace gr

#endif /* INCLUDED_SPECTRE_BATCHED_FILE_SINK_IMPL_H */
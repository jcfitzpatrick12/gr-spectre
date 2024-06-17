/* -*- c++ -*- */
/*
 * Copyright 2024 Jimmy Fitzpatrick.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SPECTRE_BATCHED_FILE_SINK_IMPL_H
#define INCLUDED_SPECTRE_BATCHED_FILE_SINK_IMPL_H

#include <gnuradio/spectre/batched_file_sink.h>
#include <gnuradio/spectre/bin_chunk_helpers.h>

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

} // namespace spectre
} // namespace gr

#endif /* INCLUDED_SPECTRE_BATCHED_FILE_SINK_IMPL_H */
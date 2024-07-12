/* -*- c++ -*- */
/*
 * Copyright 2024 Jimmy Fitzpatrick.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SPECTRE_BATCHED_FILE_SINK_IMPL_H
#define INCLUDED_SPECTRE_BATCHED_FILE_SINK_IMPL_H

#include <gnuradio/spectre/batched_file_sink.h>
#include <gnuradio/spectre/bin_chunk_helper.h>

namespace fs = std::filesystem;

namespace gr {
namespace spectre {

class batched_file_sink_impl : public batched_file_sink
{
private:
    std::string _parent_dir; // Defines the absolute path to the parent directory to hold the batched files.
    std::string _tag; // File names are tagged with this string. 
    int _chunk_size; // The (temporal) size of each batched file. 
    int _samp_rate; // The sample rate used to collect the incoming data stream. 
    bool _sweeping; // If sweeping, create a detached header file for metadata.
    std::ofstream _bin_file; // define an ofstream class to save the binary data for complex samples 
    std::ofstream _hdr_file; // define an ofstream class to save metadata if sweeping is true. 
    bool _open_new_file; // Private attribute to determine whether a new file should be opened at each call of the work function 
    float _elapsed_time; // Elapsed time since the file opening for that batch 
    bin_chunk_helper _bch; // class which handles the binary chunk file metadata 

public:
    batched_file_sink_impl(
        std::string parent_dir,
        std::string tag,
        int chunk_size,
        int samp_rate,
        bool sweeping
    ); // Constructor. 

    ~batched_file_sink_impl();// Destructor.
    void open_bin_file(); // Open a binary file ready for writing.
    void write_input_buffer_to_bin(const char* in0, int noutput_items); // Write the input buffer to the active bin file. 
    void open_hdr_file(); // Open a header file ready for writing.
    void write_metadata_to_hdr(int noutput_items); // Write the metadata information to the detached header. 

    int work(
        int noutput_items,
        gr_vector_const_void_star& input_items,
        gr_vector_void_star& output_ite
    );
};

} // namespace spectre
} // namespace gr

#endif /* INCLUDED_SPECTRE_BATCHED_FILE_SINK_IMPL_H */
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

namespace gr 
{
namespace spectre 
{
enum class file_type 
{
    BIN,
    HDR
}; // file_type is passed in as an argument to the open_file member function

class batched_file_sink_impl : public batched_file_sink
{
private:
    /* storing user inputs as member variables */
    const std::string _parent_dir; // defines the absolute path to the parent directory to hold the batched files.
    const std::string _tag; // file names are tagged with this string. 
    const int _chunk_size; // the (temporal) size of each batched file. 
    const int _samp_rate; // the sample rate used to collect the incoming data stream. 
    const bool _sweeping; // if sweeping, create a detached header file for metadata.
    /* private member variables used throughout the class. */
    std::ofstream _bin_file; // define an ofstream class to save the binary data for complex samples 
    std::ofstream _hdr_file; // define an ofstream class to save metadata if sweeping is true. 
    bool _open_new_file; // boolean determines whether new files should be opened at each call of the work function 
    float _elapsed_time; // tracks the elapsed time at each call of the work function
    bin_chunk_helper _bch; // class which offloads some of the chunk handling (computes the time stamp and file name)
    const pmt::pmt_t _frequency_key; // declare the tag key which denotes the frequency tag
    // by definition, if a frequency tag is found at absolute index N_start, and the next tag is at absolute index N_end
    // all samples with absolute index [N_start, N_end) were collected at frequency corresponding to that tag at N_start
    tag_t _active_frequency_tag; // member function to keep track of the active tag
    bool _is_active_frequency_tag_set; // boolean to determine whether the active frequency tag is set

public:
    batched_file_sink_impl(
        std::string parent_dir,
        std::string tag,
        int chunk_size,
        int samp_rate,
        bool sweeping
    ); // Constructor. 

    ~batched_file_sink_impl();// Destructor.
    void open_file(file_type ftype); // open member function for either either the binary or header file
    void ensure_first_sample_is_tagged(); // ensure the first sample is tagged when opening a new file

    // Template function for writing to the detached header
    template<typename T>
    void write_to_file(std::ofstream& file, const T* ptr_to_value, size_t size_of_value) {
        const char* bytes = reinterpret_cast<const char*>(ptr_to_value);
        file.write(bytes, size_of_value);
    }

    
    void write_tag_states_to_hdr(int noutput_items); // Write the metadata information to the detached header. 
    int work(
        int noutput_items,
        gr_vector_const_void_star& input_items,
        gr_vector_void_star& output_ite
    );
};
} // namespace spectre
} // namespace gr

#endif /* INCLUDED_SPECTRE_BATCHED_FILE_SINK_IMPL_H */
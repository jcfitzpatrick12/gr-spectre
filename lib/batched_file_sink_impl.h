/* -*- c++ -*- */
/*
 * Copyright 2024 Jimmy Fitzpatrick.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_SPECTRE_BATCHED_FILE_SINK_IMPL_H
#define INCLUDED_SPECTRE_BATCHED_FILE_SINK_IMPL_H

#include <gnuradio/spectre/batched_file_sink.h>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace gr 
{
namespace spectre 
{


// Struct used to pin down exact (defined) start time for the current batch.
struct batch_time
{
    // tm struct datetime to seconds precision.
    std::tm* utc_datetime;
    // Millisecond component.
    int64_t millisecond_component;
};


// Container which holds the file and time metadata for the batch files.
struct batch_metadata
{   
    batch_time time;
    fs::path bin_file_path;
    fs::path hdr_file_path;
};


// Classify the different batch file types.
enum class batch_file_type {
    BIN,
    HDR
};


template <typename T>
void write_to_file(std::ofstream& file, 
                   const T* data_ptr, 
                   size_t num_elements)
{
    const char* bytes = reinterpret_cast<const char*>(data_ptr);
    size_t num_bytes { num_elements * sizeof(T) };
    file.write(bytes, num_bytes);
}


class batched_file_sink_impl : public batched_file_sink
{
private:
    // User-configured member variables.
    // The parent directory which will hold the output files.
    const fs::path _parent_dir_path;
    // Identifier embedded in the output file names.
    const std::string _tag;
    // The sampling interval (inferred from the input sampling rate). 
    // I.e. the time which elapses between samples.
    const float _sampling_interval;
    // Output files are batched, each holding a specified number of seconds' worth of samples.
    const float _samples_per_batch;
    // Boolean indicates whether input stream is frequency tagged.
    const bool _is_sweeping;
    // The key of the frequency tags
    const pmt::pmt_t _frequency_tag_key;
    // The initial frequency of the incoming samples. Ignored if the first sample has a frequency tag.
    const float _initial_center_frequency;

    // Internally managed member variables
    batch_metadata _batch_metadata;
    // Output file streams for batch files.
    std::ofstream _bin_file;
    std::ofstream _hdr_file;
    // Used to infer the elapsed time at each call of the work function.
    float _sample_count;
    // Boolean indicates whether new output file streams will be created 
    // at the next call of the work function.
    bool _create_new_batch;
    // Used to record the center frequency of each sample in the input stream.
    tag_t _active_tag;
    // Default instance of a tag. Used to compare whether a tag has been set or not.
    const tag_t _default_tag; 
    // This block has only one input port.
    const int _input_port;

    static std::string get_file_extension(batch_file_type file_type);

    // Get a batch time struct based on the system clock at the time of the function call.
    static batch_time get_batch_time();

    batch_metadata get_new_batch_metadata() const;

    fs::path get_absolute_batch_file_path(std::tm* utc_datetime, 
                                          batch_file_type file_type) const;


    void write_millisecond_component_to_detached_header();

    void open_batch_file(std::ofstream& file, 
                         fs::path file_path);


    // Sets the active tag when a new batch is created
    void set_initial_active_tag();

    bool tag_is_set(tag_t tag) const;
    bool initial_center_frequency_is_set() const;
    std::optional<tag_t> get_tag_from_first_sample();

    // Get the number of samples per center frequency via frequency tag offsets.
    // Consider two neighbouring frequency tags, tag_i and tag_j, with offsets i and j (i < j) 
    // All samples z_k, with k in [i, j), correspond to the frequency tag_i.value
    std::vector<float> get_num_samples_per_center_frequency(int noutput_items);
   
    void initialise_new_batch();


public:
    batched_file_sink_impl(const std::string parent_dir_path,
                           const std::string tag,
                           const float batch_size,
                           const int samp_rate,
                           const bool is_sweeping,
                           const std::string frequency_tag_key,
                           const float initial_center_frequency);
    ~batched_file_sink_impl();

    // Where all the action really happens.
    int work(int noutput_items,
             gr_vector_const_void_star& input_items,
             gr_vector_void_star& output_items);
};

} // namespace spectre
} // namespace gr

#endif /* INCLUDED_SPECTRE_BATCHED_FILE_SINK_IMPL_H */
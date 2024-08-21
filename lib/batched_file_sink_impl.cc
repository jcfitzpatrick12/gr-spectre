/* -*- c++ -*- */
/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "batched_file_sink_impl.h"
#include <gnuradio/io_signature.h>

namespace gr {
namespace spectre {

using input_type = gr_complex;

batched_file_sink::sptr batched_file_sink::make(std::string parent_dir,
                                                std::string tag,
                                                int chunk_size,
                                                int samp_rate,
                                                bool sweeping,
                                                std::string frequency_tag_key,
                                                float initial_active_frequency) {
    return gnuradio::make_block_sptr<batched_file_sink_impl>(
        parent_dir, tag, chunk_size, samp_rate, sweeping, frequency_tag_key, initial_active_frequency);
}

batched_file_sink_impl::batched_file_sink_impl(std::string parent_dir,
                                               std::string tag,
                                               int chunk_size,
                                               int samp_rate,
                                               bool sweeping,
                                               std::string frequency_tag_key,
                                               float initial_active_frequency)
    : gr::sync_block("batched_file_sink",
                     gr::io_signature::make(1, 1, sizeof(input_type)),
                     gr::io_signature::make(0, 0, 0)),
      _parent_dir(std::move(parent_dir)), /**< user input */
      _tag(std::move(tag)), /**< user input */
      _chunk_size(chunk_size),/**< user input */
      _samp_rate(samp_rate), /**< user input */
      _sweeping(sweeping), /**< user input */
      _open_new_file(true), // impose that we will open a new file at the first call of the work function
      _elapsed_time(0), // elapsed time is zero initially, by definition.
      _bch(_parent_dir, _tag), // create an instance of the bin chunk handler class
      _frequency_tag_key(pmt::string_to_symbol(frequency_tag_key)), // declaring the frequency tag key
      _is_active_frequency_tag_set(false), // the active frequency tag is not set until the first sample in the stream
      _initial_active_frequency(initial_active_frequency) // if a tag is not available on the very first sample, this float is used by default
{
}

batched_file_sink_impl::~batched_file_sink_impl() {
    if (_bin_file.is_open()) {
        _bin_file.close();
    }

    if (_hdr_file.is_open()) {
        _hdr_file.close();
    }
}


void batched_file_sink_impl::open_file(file_type ftype) {
    std::ofstream* file = nullptr;
    fs::path file_path;

    // Determine file type and set file path
    switch (ftype) {
        case file_type::BIN:
            file = &_bin_file;
            file_path = _bch.get_bin_absolute_path();
            break;
        case file_type::HDR:
            file = &_hdr_file;
            file_path = _bch.get_hdr_absolute_path();
            break;
        default:
            throw std::invalid_argument("Invalid file type specified.");
    }

    // Close the file if already open
    if (file->is_open()) {
        file->close();
    }

    // Create necessary directories if they don't exist
    fs::path parent_path_with_date_dirs = _bch.get_parent_path_with_date_dirs();
    if (!fs::exists(parent_path_with_date_dirs)) {
        try {
            fs::create_directories(parent_path_with_date_dirs);
        } catch (const fs::filesystem_error& e) {
            throw std::runtime_error("Failed to create directories: " + parent_path_with_date_dirs.string() + ", error: " + e.what());
        }
    }

    // Open the file for writing
    file->open(file_path, std::ios::binary | std::ios::out);
    if (!file->is_open()) {
        throw std::runtime_error("Failed to open file: " + file_path.string());
    }
}
void batched_file_sink_impl::set_initial_active_frequency_tag() 
{    
    int first_sample_abs_index = nitems_read(0);
    // now populate a vector, v, which will contain one element if the first sample is tagged, and is empty otherwise
    std::vector<tag_t> vector_wrapped_first_sample_tag;
    get_tags_in_range(vector_wrapped_first_sample_tag, 0, first_sample_abs_index, first_sample_abs_index + 1, _frequency_tag_key);
    // inspect whether the first sample has a tag or not
    bool first_sample_has_tag = !vector_wrapped_first_sample_tag.empty();

    // if the active frequency has already been set, update the offset to coincide with the first sample as of this call to the work function
    if (_is_active_frequency_tag_set) 
    {
        _active_frequency_tag.offset = nitems_read(0);
    }
    // otherwise the active frequency has not yet been set
    else 
    {
        // if the user has explicitly specified the initial active frequency (i.e. it is non-zero) 
        // use this to define the initial active frequency tag 
        if (_initial_active_frequency != 0) {
            tag_t _active_frequency_tag;
            uint64_t initial_offset = 0;
            _active_frequency_tag.offset = initial_offset;
            _active_frequency_tag.key = _frequency_tag_key;
            _active_frequency_tag.value = pmt::from_float(_initial_active_frequency);
            _active_frequency_tag.srcid = pmt::intern(alias());  // Set the srcid using the block's alias
        }

        // if the first sample has a tag, override the active frequency with that attached to the 
        // first sample
        else if (first_sample_has_tag)
        {
            _active_frequency_tag = vector_wrapped_first_sample_tag[0];
        }

        // Otherwise, we have an undefined tag state.
        /*
        Why is this undefined? We need to know the frequency corresponding to each IQ sample in the stream. 
        Consider two neighbouring frequency tags, tag_i and tag_j, with absolute indices i and j, where i < j. 
        All samples z_k, with k in [i, j), correspond to the frequency tag_i.value. 

        Therefore, for any IQ sample z_k in the stream, there exists a tag tag_i where i <= k, which we call the 
        active tag for z_k. From this, the first sample (z_0) must have a corresponding tag at absolute index 0. 
        If _is_active_frequency_tag_set is false (i.e. during the first call of the work function) and the first sample 
        is not tagged (and no additional information has been provided w.r.t. the initial state) we cannot determine 
        its corresponding frequency, leading to an undefined state.
        */
        else
        {
            throw std::runtime_error("Undefined tag state, the first sample in the stream must be frequency tagged!");
        }
    }
    // update member variable to note that the active tag is now set
    _is_active_frequency_tag_set = true;
}


void batched_file_sink_impl::write_tag_states_to_hdr(int noutput_items) {
    // search for tags subsequent to the current active tag
    int abs_start_index = _active_frequency_tag.offset + 1;
    // up until the current range of the work function
    int abs_end_index = nitems_read(0) + noutput_items;
    // Vector to hold all tags in the current range of the work function
    std::vector<tag_t> frequency_tags;
    get_tags_in_range(frequency_tags, 0, abs_start_index, abs_end_index, _frequency_tag_key);

    // Iterate through each tag and compute the number of samples for each tag interval
    for (const tag_t& frequency_tag : frequency_tags) {
        // Compute the number of samples then update the active tag
        int32_t num_samples_active_frequency = frequency_tag.offset - _active_frequency_tag.offset;
        // Cast as a float (for ease of reading in post-processing)
        float num_samples_active_frequency_as_float = static_cast<float>(num_samples_active_frequency);

        // Ensure the PMT value is numeric before attempting conversion
        if (pmt::is_number(_active_frequency_tag.value)) {
            float active_frequency = static_cast<float>(pmt::to_double(_active_frequency_tag.value));

            // Write the active frequency and the number of samples to the header file
            write_to_file(_hdr_file, &active_frequency, sizeof(float));
            write_to_file(_hdr_file, &num_samples_active_frequency_as_float, sizeof(float));
        } else {
            throw std::runtime_error("Active frequency tag value is not numeric.");
        }

        // Update the active frequency
        _active_frequency_tag = frequency_tag;
    }

    // If _open_new_file is set to true, then we will be opening a new file at the next call of the work function.
    // So we need to do some clean-up in this case
    if (_open_new_file) {
        /*
        Essentially, at this moment we have handled all the tags in the current call of the work function. 
        Thus, there is no "next tag" to evaluate the total number of samples associated with the current active tag. 
        For a clean handover to the next file, we compute the number of samples REMAINING at the active tag in the current call of the work function.
        
        In this way, we will have declared for every IQ sample dumped to the bin file, what frequency it was collected at.
        */
        // Compute the number of remaining samples and write to the header file ...
        int32_t num_samples_remaining = (nitems_read(0) + noutput_items) - _active_frequency_tag.offset;
        // Cast as a float (for ease of reading in post-processing)
        float num_samples_remaining_as_float = static_cast<float>(num_samples_remaining);

        // Ensure the PMT value is numeric before attempting conversion
        if (pmt::is_number(_active_frequency_tag.value)) {
            float active_frequency = static_cast<float>(pmt::to_double(_active_frequency_tag.value));

            // Write this to file
            write_to_file(_hdr_file, &active_frequency, sizeof(float));
            write_to_file(_hdr_file, &num_samples_remaining_as_float, sizeof(float));
        } else {
            throw std::runtime_error("Active frequency tag value is not numeric.");
        }

        // Don't update the active frequency, as we will initialise this at the next call of the work function
    }
}


int batched_file_sink_impl::work(
    int noutput_items,
    gr_vector_const_void_star& input_items,
    gr_vector_void_star& output_items
) {
    if (_open_new_file) {
        _open_new_file = false; // ensure we won't open another file until _open_new_file is set back to true
        _bch.update(); // effectively set the header and binary file paths, and compute the ms correction
        open_file(file_type::BIN); // open the binary file ready for the raw IQ samples
        open_file(file_type::HDR); // open the detached header ready for metadata writing
        int32_t millisecond_correction = _bch.get_millisecond_correction(); // extract the millisecond correction 
        float millisecond_correction_as_float = static_cast<float>(millisecond_correction); // convert to float (for consistency when reading, we will check its integerness when reading)
        write_to_file(_hdr_file, &millisecond_correction_as_float, sizeof(float)); // and write to the hdr file
        _elapsed_time = 0; // Reset elapsed time when a new file is opened
        
        if (_sweeping) {
            set_initial_active_frequency_tag();
        }
    }

    /* dump the contents input buffer to the bin file */
    write_to_file(_bin_file, input_items[0], sizeof(gr_complex) * noutput_items);

    /* inferring elapsed time based on the number of samples processed */
    _elapsed_time += noutput_items * (1.0 / _samp_rate);
    // If elapsed time is greater than the (input) chunk_size, at the next iteration, we will open a new file
    if (_elapsed_time >= _chunk_size) {
        _open_new_file = true;
    }

    /* if the sweeping flag is true, write the frequency tag information to the detached header. */
    if (_sweeping) {
        write_tag_states_to_hdr(noutput_items);
    }
    return noutput_items;
}

} /* namespace spectre */
} /* namespace gr */

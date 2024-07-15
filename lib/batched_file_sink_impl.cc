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
                                                bool sweeping) {
    return gnuradio::make_block_sptr<batched_file_sink_impl>(
        parent_dir, tag, chunk_size, samp_rate, sweeping);
}

batched_file_sink_impl::batched_file_sink_impl(std::string parent_dir,
                                               std::string tag,
                                               int chunk_size,
                                               int samp_rate,
                                               bool sweeping)
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
      _frequency_key(pmt::string_to_symbol("rx_freq")), // declaring the frequency tag key
      _is_active_frequency_tag_set(false) // the active frequency tag is not set until the first sample in the stream
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
void batched_file_sink_impl::ensure_first_sample_is_tagged() 
{      
    // // at this particular call of the work function, find the absolute start index of the first sample
    // uint64_t abs_start_N = nitems_read(0);
    // // now populate a vector which will contain one element if the first sample is tagged, and is empty otherwise
    // uint64_t abs_end_N = abs_start_N + 1;
    // std::vector<tag_t> vector_frequency_tag_of_first_sample;
    // get_tags_in_range(vector_frequency_tag_of_first_sample, 0, abs_start_N, abs_end_N, _frequency_key);
    
    // if (vector_frequency_tag_of_first_sample.empty())
    // {
    //     if (_is_active_frequency_tag_set)
    //     {
    //         const uint64_t absolute_offset = nitems_read(0);
    //         const pmt::pmt_t key = _frequency_key;
    //         const pmt::pmt_t value = _active_frequency_tag.value;
    //         const pmt::pmt_t srcid = pmt::string_to_symbol(alias());
    //         add_item_tag(0, absolute_offset, key, value, srcid);
    //     }
    //     else
    //     {
    //        throw std::runtime_error("First sample in the stream must be frequency tagged!");
    //     }
    // }

    // // otherwise the first sample is tagged, and we have nothing to do!
}




void batched_file_sink_impl::write_tag_states_to_hdr(int noutput_items) {  
    // // Compute the absolute start and end indices
    // // we know the first tag is sampled, so skip that!
    // uint64_t abs_start_N = nitems_read(0) + 1;
    // // and infer the absolute end index from the number of output_items considered at this call of the work function
    // uint64_t abs_end_N = abs_start_N + noutput_items;

    // // Vector to hold all tags in the current range
    // std::vector<tag_t> frequency_tags;
    // get_tags_in_range(frequency_tags, 0, abs_start_N, abs_end_N, _frequency_key);
    
    // // Iterate through each tag and compute the number of samples for each tag interval
    // for (const tag_t &frequency_tag : frequency_tags) {
    //     // Compute the number of samples then update the active tag
    //     int32_t num_samples_active_frequency = frequency_tag.offset - _active_frequency_tag.offset;
    //     // Compute the active frequency 
    //     float active_frequency = pmt::to_float(_active_frequency_tag.value);

    //     std::cout << "Active frequency: " << active_frequency << std::endl;
    //     std::cout << "Num samples: " << num_samples_active_frequency <<std::endl;

    //     // and write to file
    //     write_to_file(_hdr_file, &active_frequency, sizeof(float));
    //     write_to_file(_hdr_file, &num_samples_active_frequency, sizeof(int32_t));
    //     // Update the active frequency
    //     _active_frequency_tag = frequency_tag;
    // }

    // // if _open_new_file is set to true, we need to do some clean-up before opening the next file at the next call of the work function
    // if (_open_new_file) 
    // {
    //     // compute the number of remaining samples and write to the header file ...
    //     int32_t num_samples_remaining = (nitems_written(0) + noutput_items) - _active_frequency_tag.offset;
    //     // Compute the active frequency 
    //     float active_frequency = pmt::to_float(_active_frequency_tag.value);
    //     std::cout << "Dangling frequency: " << active_frequency << std::endl;
    //     std::cout << "Samples remaining: " << num_samples_remaining <<std::endl;
    //     // write this to file
    //     write_to_file(_hdr_file, &active_frequency, sizeof(float));
    //     write_to_file(_hdr_file, &num_samples_remaining, sizeof(int32_t));
    //     // don't update the active frequency, as we will use this during inspect_first_sample at the next call of the work function

    // }
}

int batched_file_sink_impl::work(
    int noutput_items,
    gr_vector_const_void_star& input_items,
    gr_vector_void_star& output_items
) {  
    std::cout << nitems_read(0) << std::endl;
    std::cout << nitems_written(0) << std::endl;

    if (_open_new_file) {
        _open_new_file = false; // ensure we won't open another file until _open_new_file is set back to true
        _bch.update(); // effectively set the header and binary file paths, and compute the ms correctione
        open_file(file_type::BIN); // open the binary file ready for the raw IQ samples
        open_file(file_type::HDR); // open the detached header ready for metadata writing
        int32_t millisecond_correction = _bch.get_millisecond_correction(); // extract the millisecond correction and write to the detached header
        write_to_file(_hdr_file, &millisecond_correction, sizeof(int32_t)); 
        ensure_first_sample_is_tagged(); // inspect the first sample at the first call of the work function after opening a new file
    }

    /* dump the contents input buffer to the bin file */
    write_to_file(_bin_file, input_items[0], sizeof(gr_complex)*noutput_items);

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

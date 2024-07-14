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
      _parent_dir(parent_dir), /**< user input */
      _tag(tag), /**< user input */
      _chunk_size(chunk_size),/**< user input */
      _samp_rate(samp_rate), /**< user input */
      _sweeping(sweeping), /**< user input */
      _open_new_file(true), // impose that we will open a new file at the first call of the work function
      _elapsed_time(0), // elapsed time is zero initially, by definition.
      _bch(parent_dir, tag), // create an instance of the bin chunk handler class
      _frequency_key(pmt::string_to_symbol("rx_freq")), // declaring the frequency tag key
      _is_active_frequency_tag_set(false) // at the first call of the work function, the first tag has not yet been set
{
};

batched_file_sink_impl::~batched_file_sink_impl(
)
{
if (_bin_file.is_open()) {
    _bin_file.close();
};

if (_hdr_file.is_open()) {
    _hdr_file.close();
}
};


void batched_file_sink_impl::open_file(file_type ftype) 
{
    std::ofstream* file = nullptr;
    fs::path file_path;

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

    if (file->is_open()) {
        file->close();
    }

    if (!fs::exists(_bch.get_parent_path_with_date_dirs())) {
        bool created = fs::create_directories(_bch.get_parent_path_with_date_dirs());
        if (!created) {
            throw std::runtime_error("Failed to create directories: " + _bch.get_parent_path_with_date_dirs().string());
        }
    }

    file->open(file_path, std::ios::binary | std::ios::out);

    if (!file->is_open()) {
        throw std::runtime_error("Failed to open file: " + file_path.string());
    }
}


void batched_file_sink_impl::write_ms_correction_to_hdr() 
{
    int32_t millisecond_correction = _bch.get_millisecond_correction();
    const char* millisecond_correction_bytes = reinterpret_cast<char*>(&millisecond_correction);
    _hdr_file.write(millisecond_correction_bytes, sizeof(int32_t));
}


void batched_file_sink_impl::write_input_buffer_to_bin(
    const char* in0,
    int noutput_items
)
{
    /* write the entire buffer to the current active binary chunk file */
    _bin_file.write(in0, noutput_items * sizeof(gr_complex));
    // infer the elapsed_time from the (input) sample rate, and number of items in the buffer
    _elapsed_time += noutput_items * (1.0 / _samp_rate);
    // if elapsed time is greater than the (input) chunk_size, at the next iteration, we will open a new file
    if (_elapsed_time >= _chunk_size) {
        _open_new_file = true;
    }
}


void batched_file_sink_impl::write_active_frequency_to_hdr(float frequency)
{
    // to be implemented
}


void batched_file_sink_impl::write_num_samples_to_hdr(int32_t num_samples)
{
    // to be implemented
}


void batched_file_sink_impl::write_tag_states_to_hdr(int noutput_items) {  
    // Compute the absolute start and end indices
    uint64_t abs_start_N = nitems_read(0);
    uint64_t abs_end_N = abs_start_N + (uint64_t)(noutput_items);

    // Vector to hold all tags in the current range
    std::vector<tag_t> frequency_tags;
    get_tags_in_range(frequency_tags, 0, abs_start_N, abs_end_N, _frequency_key);
    
    // Iterate through each tag and compute the number of samples for each tag interval
    for (const tag_t &frequency_tag : frequency_tags) {
        // if the first tag has not been set... initialise the active frequency tag!
        if (!_is_active_frequency_tag_set) {
            // set the active frequency tag (ensuring that the first tagged sample is the first sample of the stream)
            if (frequency_tag.offset == 0) {
                _active_frequency_tag = frequency_tag;
                _is_active_frequency_tag_set = true;
            }
            else {
                throw std::runtime_error("The first sample of the stream must be tagged with the frequency.");
            }
        }
        // otherwise we have an active frequency tag
        // so, we can compute the number of samples then update the active tag
        else {
            // we have an initial tag, so we can compute the number of elements
            int32_t num_samples_active_frequency = frequency_tag.offset - _active_frequency_tag.offset;
            // write the active frequency to the hdr
            write_active_frequency_to_hdr(pmt::to_float(_active_frequency_tag.value));
            write_num_samples_to_hdr(num_samples_active_frequency);

            // print check
            std::cout << "Active frequency: " << pmt::to_float(_active_frequency_tag.value) << std::endl;
            std::cout << "Active frequency offset: " <<  _active_frequency_tag.offset << std::endl;

            // update the active frequency
            _active_frequency_tag = frequency_tag;

        }
    }
}


int batched_file_sink_impl::work(
    int noutput_items,
    gr_vector_const_void_star& input_items,
    gr_vector_void_star& output_items
){  
    
    /* For the batched file sink block, input_items is a vector 
    of constant void pointers of length one,
    where the only element points to the first element of the 
    buffer of complex samples. In the conversion below, we instead 
    cast the void pointer as a character stream in order to 
    access the raw bytes which we will in the proceeding save to file */
    const char* in0 = static_cast<const char*>(input_items[0]);

    if (_open_new_file) {
        _bch.update(); // effectivly set the header and binary file paths, and compute the ms correction
        _open_new_file = false; // impose that we won't open another file until _open_new_file is set back to true
        open_file(file_type::BIN); // open the binary file ready for the raw IQ samples
        open_file(file_type::HDR); // open the detached header ready for metadata writing
        write_ms_correction_to_hdr();
    }

    write_input_buffer_to_bin(in0, noutput_items);

    // if sweeping is activated, we will save the tag information in the detached header
    if (_sweeping) {
        write_tag_states_to_hdr(noutput_items);
    }
    return noutput_items;
};

} /* namespace spectre */
} /* namespace gr */

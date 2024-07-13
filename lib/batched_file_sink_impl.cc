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
      _parent_dir(parent_dir), // instantiate with user input.
      _tag(tag), // instantiate with user input.
      _chunk_size(chunk_size), // instantiate with user input.
      _samp_rate(samp_rate), // instantiate with user input.
      _sweeping(sweeping), // instantiate with user input.
      _open_new_file(true), // impose that we will open a new file when this class is instantiate
      _elapsed_time(0), // elapsed time is zero initially, by definition.
      _bch(parent_dir, tag), // create an instance of the bin chunk handler class
      _frequency_key(pmt::string_to_symbol("rx_freq")), // define here what the frequency tag is
      _abs_index_last_tag(0) // by assumption, we assume that the first sample is tagged
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


void batched_file_sink_impl::open_file(file_type ftype) {
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


void batched_file_sink_impl::write_ms_correction() 
{
    int32_t millisecond_correction = _bch.get_millisecond_correction();
    const char* millisecond_correction_bytes = reinterpret_cast<char*>(&millisecond_correction);

    if (_hdr_file.is_open()) {
        _hdr_file.write(millisecond_correction_bytes, sizeof(int32_t));
    } else {
        throw std::runtime_error("Failed to write ms_correction to bin file.");
    }
}


void batched_file_sink_impl::write_input_buffer_to_bin(
    const char* in0,
    int noutput_items
){
    // write the entire buffer to the current active binary chunk file
    _bin_file.write(in0, noutput_items * sizeof(gr_complex));
    // infer the elapsed_time from the (input) sample rate, and number of items in the buffer
    _elapsed_time += noutput_items * (1.0 / _samp_rate);
    // if elapsed time is greater than the (input) chunk_size, at the next iteration, we will open a new file
    if (_elapsed_time >= _chunk_size) {
        _open_new_file = true;
    }
}

void batched_file_sink_impl::ensure_first_sample_tagged() {
    // prepare a vector to hold all the tags in the current range of the work functio
    std::vector<tag_t> first_tag;
    // essentially fetch a vector which contains the tag of the first sample if it exists, and is empty otherwise
    get_tags_in_range(first_tag, 0, 0, 1);
    // if the first sample is not tagged, throw a runtime error
    if (first_tag.empty()) {
        throw std::runtime_error("First sample is missing a tag!");
    }
}

void batched_file_sink_impl::write_metadata_to_hdr(
    int noutput_items
){  
    // find how many items have been read as of all previous calls to the work function
    uint64_t abs_start_N = nitems_read(0);
    // compute how any items are being processed by the work function currently
    uint64_t abs_end_N = abs_start_N + (uint64_t)(noutput_items);
    // prepare a vector to hold all the tags in the current range of the work functio
    std::vector<tag_t> all_tags;
    // get all frequency tags
    get_tags_in_range(all_tags, 0, abs_start_N, abs_end_N, _frequency_key);
    // loop through each tag
    for (const tag_t &tag : all_tags) {
        // find the current frequency
        _current_frequency = pmt::to_float(tag.value);
        // find the current tag offset 
        _abs_index_last_tag = tag.offset;
        // print the absolute index of each tag
        std::cout << _abs_index_last_tag << std::endl;
        // compute the sample offset (tag index displaced s.t. start_N is at index zero)
        // int sample_index = static_cast<int>(tag.offset - abs_start_N);
    }

    // write the stream ()
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
        _bch.update(); // effectivly set the header and binary file paths, and computes ms correction
        _elapsed_time = 0; // set the elapsed time to zero, 
        _open_new_file = false; // impose that we won't open another file until _open_new_file is set back to true
        open_file(file_type::BIN); // open the binary file ready for the raw IQ samples
        open_file(file_type::HDR); // open the detached header ready for the ms_correction and (if sweeping) swept tag info
        write_ms_correction(); // write the ms_correction to the detached header
    }

    // whether or not we saving tagged metadata in the detached header, dump all the IQ samples to the bin chunk
    if (_bin_file.is_open()) {
        write_input_buffer_to_bin(in0, noutput_items);
    }
    else {
        throw std::runtime_error("Failed to write to bin file.");
    }

    // if sweeping is activated, save the tagged info in the detached header file
    if (_sweeping) {
        // if this is our first call to the work function, check the first sample is tagged
        int num_items_read = nitems_read(0);
        if (num_items_read == 0) {
            ensure_first_sample_tagged();
        }
        // if it is, proceed as normal
        if (_hdr_file.is_open()){
            write_metadata_to_hdr(noutput_items);
        }
        else {
            throw std::runtime_error("Failed to write to hdr file.");
        }
        
    }
    return noutput_items;
};

} /* namespace spectre */
} /* namespace gr */

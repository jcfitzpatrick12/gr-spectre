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
      _bch(parent_dir, tag) // create an instance of the bin chunk handler class
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

void batched_file_sink_impl::open_bin_file() 
{   
    if (_bin_file.is_open()) {
            _bin_file.close();
        };

    /* (essentially) update then fetch the _absolute_path and _millisecond 
    correction member variables of the instance _bch of the batched_file_handler class */
    _bch.set_attrs();
    fs::path bin_chunk_path = _bch.get_bin_absolute_path();
    fs::path parent_path_with_date_dirs = _bch.get_parent_path_with_date_dirs();

    // if the parent path does not already exist, create it
    if (!fs::exists(parent_path_with_date_dirs)) {
        fs::create_directories(parent_path_with_date_dirs);
    };
    
    // open the batched file in binary mode, ready for writing.
    _bin_file.open(bin_chunk_path, std::ios::binary | std::ios::out);

    // extract the millisecond correction
    int32_t millisecond_correction = _bch.get_millisecond_correction();
    // Convert the integer to a char pointer to access its raw byte representation
    const char* millisecond_correction_bytes = reinterpret_cast<char*>(&millisecond_correction);
    // write the millisecond correction to the start of the binary file
    if (_bin_file.is_open()) {
        _bin_file.write(millisecond_correction_bytes, sizeof(int));
    }
    else {
        throw std::runtime_error("Failed to open file.");
    };

    /* once a file has been opened, we will not open a new file until chunk_time [s] have elapsed */
    _elapsed_time = 0; 
    _open_new_file = false;
};

void batched_file_sink_impl::open_hdr_file() {
    // this is called after open_bin_file, and we want to preserve the time stamp,
    // so use the member variable _bch assuming set_attrs has already occurred
    fs::path hdr_chunk_path = _bch.get_hdr_absolute_path();
    fs::path parent_path = _bch.get_parent_path_with_date_dirs();

    // if the parent path does not already exist, create it
    if (!fs::exists(parent_path)) {
        fs::create_directories(parent_path);
    };
    
    // open the hdr batched file in binary mode, ready for writing.
    _hdr_file.open(hdr_chunk_path, std::ios::binary | std::ios::out);
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

void batched_file_sink_impl::write_metadata_to_hdr(
    int noutput_items
){  
    // find how many items have been read as of all previous calls to the work function
    uint64_t abs_N = nitems_read(0);
    // compute how any items are being processed by the work function currently
    uint64_t end_N = abs_N + (uint64_t)(noutput_items);
    std::vector<tag_t> all_tags;
    // can specify the key here to extract tags of a particular key, likely specify 
    get_tags_in_range(all_tags, 0, abs_N, end_N /*, pmt::pmt_t key*/);
    // loop through each tag
    for (const tag_t &tag : all_tags) {
        std::cout << pmt::symbol_to_string(tag.key) << std::endl;
        // evaluate the offsets between each tag and thus compute the number of items between each new tag
        // keep track of this as part of a member variable somehow
        // so to allow continuity between calls to the work function and also opening new files
        // as we go, add the bytes to hdr_bytes vector!
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
        open_bin_file();
        // if sweeping is configured true, open a hdr file for the tagged metadata
        if (_sweeping) {
            open_hdr_file();
        }
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

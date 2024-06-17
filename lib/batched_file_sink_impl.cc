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
                                                int samp_rate) {
    return gnuradio::make_block_sptr<batched_file_sink_impl>(
        parent_dir, tag, chunk_size, samp_rate);
}

batched_file_sink_impl::batched_file_sink_impl(std::string parent_dir,
                                               std::string tag,
                                               int chunk_size,
                                               int samp_rate)
    : gr::sync_block("batched_file_sink",
                     gr::io_signature::make(1, 1, sizeof(input_type)),
                     gr::io_signature::make(0, 0, 0)),
      _parent_dir(parent_dir),
      _tag(tag),
      _chunk_size(chunk_size),
      _samp_rate(samp_rate),
      _open_new_file(true),
      _elapsed_time(0),
      _bch(parent_dir, tag) 
{
    
};

batched_file_sink_impl::~batched_file_sink_impl(
)
{
if (_file.is_open()) {
    _file.close();
};
};

void batched_file_sink_impl::open_file() 
{   
    if (_file.is_open()) {
            _file.close();
        };

    /* (essentially) update then fetch the _absolute_path and _millisecond 
    correction member variables of the instance _bch of the batched_file_handler class */
    _bch.set_attrs();
    fs::path bin_chunk_path = _bch.get_absolute_path();
    fs::path parent_path = _bch.get_absolute_parent_path();

    // if the parent path does not already exist, create it
    if (!fs::exists(parent_path)) {
        fs::create_directories(parent_path);
    };
    
    // open the batched file in binary mode, ready for writing.
    _file.open(bin_chunk_path, std::ios::binary | std::ios::out);

    // extract the millisecond correction
    int32_t millisecond_correction = _bch.get_millisecond_correction();
    // Convert the integer to a char pointer to access its raw byte representation
    const char* millisecond_correction_bytes = reinterpret_cast<char*>(&millisecond_correction);
    // write the millisecond correction to the start of the binary file
    if (_file.is_open()) {
        _file.write(millisecond_correction_bytes, sizeof(int));
    }
    else {
        throw std::runtime_error("Failed to open file.");
    };

    /* once a file has been opened, we will not open a new file until chunk_time [s] have elapsed */
    _elapsed_time = 0; 
    _open_new_file = false;
};

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
        open_file();
    }

    if (_file.is_open()) {
        // write the entire buffer to the current active binary chunk file
        _file.write(in0, noutput_items * sizeof(gr_complex));
        // infer the elapsed_time from the (input) sample rate, and number of items in the buffer
        _elapsed_time += noutput_items * (1.0 / _samp_rate);
        // if elapsed time is greater than the (input) chunk_size, at the next iteration, we will open a new file
        if (_elapsed_time >= _chunk_size) {
            _open_new_file = true;
        }
    } else {
        throw std::runtime_error("Failed to write to file.");
    }
    return noutput_items;
};

} /* namespace spectre */
} /* namespace gr */

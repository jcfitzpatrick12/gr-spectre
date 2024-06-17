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


bin_chunk_helper::bin_chunk_helper(
    std::string parent_path,
    std::string tag
){
    _parent_path = fs::path(parent_path);
    _tag = tag;
};

bin_chunk_helper::~bin_chunk_helper(){};

void bin_chunk_helper::set_attrs(){
    // evaluate current system time as ms since epoch (expressed as a 64 bit integer)
    auto now = std::chrono::system_clock::now();
    auto time_since_epoch = now.time_since_epoch();
    auto ms_duration_since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(time_since_epoch);
    int64_t ms_since_epoch = ms_duration_since_epoch.count();

    // infer the (floored) seconds since epoch and the ms_remainder
    // s.t. floored_seconds_since_epoch [s] + ms_remainder [s] = ms_since_epoch [s]
    int64_t floored_seconds_since_epoch = ms_since_epoch / 1000;
    // convert to int32_t for consistency when we save it to the binary file
    int32_t ms_remainder = ms_since_epoch % 1000;
    
    // cast seconds_since_epoch as a pointer to an equivalent tm struct in UTC
    std::tm* current_gmt = std::gmtime(&floored_seconds_since_epoch);

    // Format the date path and file name
    std::stringstream date_path_ss, bin_chunk_name_ss, absolute_path_ss;
    date_path_ss << std::put_time(current_gmt, "%Y/%m/%d");
    bin_chunk_name_ss << std::put_time(current_gmt, "%Y-%m-%dT%H:%M:%S") << "_" << _tag << ".bin";

    // convert to path objects
    fs::path date_path, bin_chunk_name;
    date_path = fs::path(date_path_ss.str());
    bin_chunk_name = fs::path(bin_chunk_name_ss.str());

    // and finally, set the appropriate member variables
    _absolute_path = _parent_path / date_path / bin_chunk_name;
    _millisecond_correction = ms_remainder;
};

fs::path bin_chunk_helper::get_absolute_path(){
    return _absolute_path;
};


fs::path bin_chunk_helper::get_absolute_parent_path(){
    return _absolute_path.parent_path();
};

} /* namespace spectre */
} /* namespace gr */

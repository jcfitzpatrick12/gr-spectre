/* -*- c++ -*- */
/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "batched_file_sink_impl.h"
#include <gnuradio/io_signature.h>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

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
                     gr::io_signature::make(0, 0, 0))
{
    _parent_dir = parent_dir;
    _tag = tag;
    _samp_rate = samp_rate;
    _chunk_size = chunk_size;

    _open_new_file = true;
    _elapsed_time = 0;
}

batched_file_sink_impl::~batched_file_sink_impl() {
    _file.close(); // Ensure file is closed
}

int batched_file_sink_impl::work(int noutput_items,
                                 gr_vector_const_void_star& input_items,
                                 gr_vector_void_star& output_items) {
    if (_open_new_file) {
        open_file();
    }

    const char* in = static_cast<const char*>(input_items[0]);

    if (_file.is_open()) {
        _file.write(in, noutput_items * sizeof(gr_complex));
        _elapsed_time += noutput_items * (1.0 / _samp_rate);

        if (_elapsed_time >= _chunk_size) {
            _open_new_file = true;
        }
    } else {
        throw std::runtime_error("Failed to write to file.");
    }
    return noutput_items;
}

void batched_file_sink_impl::open_file() {
    if (_file.is_open()) {
        _file.close();
    }

    fs::path file_path = make_file_path(_parent_dir, _tag);
    _file.open(file_path, std::ios::binary | std::ios::out); // Correct use of _file

    if (!_file) {
        throw std::runtime_error("Failed to open file at " + file_path.string());
    }
    
    _open_new_file = false;
    _elapsed_time = 0; 
}

fs::path batched_file_sink_impl::make_file_path(const fs::path& parent_dir, const std::string& tag) {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::gmtime(&time_t);

    std::stringstream dir_ss, file_ss;
    dir_ss << std::put_time(&tm, "%Y/%m/%d");
    file_ss << std::put_time(&tm, "%Y%m%dT%H%M%S%f") << "_" << tag << ".bin";

    fs::path dir_path = parent_dir / dir_ss.str();
    if (!fs::exists(dir_path)) {
        if (!fs::create_directories(dir_path)) {
            throw std::runtime_error("Failed to create directory: " + dir_path.string());
        }
    }

    return dir_path / file_ss.str();
}

} /* namespace spectre */
} /* namespace gr */

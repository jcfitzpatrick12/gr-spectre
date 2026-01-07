/*
 * Copyright 2024-2026 Jimmy Fitzpatrick.
 * This file is part of SPECTRE
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "batched_file_sink_impl.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>

namespace {

static constexpr int NUM_DIGITS_MILLISECONDS = 3;

int get_sizeof_stream_item(const std::string& input_type)
{
    // Get the size of each item in the input stream, in bytes.
    if (input_type == "fc32") {
        return sizeof(gr_complex);
    } else if (input_type == "fc64") {
        return sizeof(gr_complexd);
    } else if (input_type == "sc16") {
        return sizeof(int16_t[2]);
    } else if (input_type == "sc8") {
        return sizeof(int8_t[2]);
    } else {
        throw std::invalid_argument("Unsupported input type: " + input_type);
    }
}

int get_num_samples_per_batch(const float batch_size, const int sample_rate)
{
    // Naturally, we can't have a non-integral number of samples in a batch,
    // so we floor to ensure that the elapsed time per batch doesn't surpass the
    // user-configured batch size.
    return std::floor(batch_size * sample_rate);
}

std::filesystem::path generate_file_path(const std::string& dir,
                                         const std::string& tag,
                                         const std::string& extension,
                                         const bool group_by_date,
                                         std::tm utc_tm,
                                         int ms)
{
    // Make the file path, embedding the current system time if required.
    std::ostringstream buffer;
    std::string fmt = (group_by_date) ? "%Y/%m/%d/" : "";
    buffer << std::put_time(&utc_tm, fmt.data())
           << std::put_time(&utc_tm, "%Y-%m-%dT%H:%M:%S.")
           << std::setw(NUM_DIGITS_MILLISECONDS) << std::setfill('0') << ms << "Z_" << tag
           << "." << extension;
    return dir / std::filesystem::path(buffer.str());
}

} // namespace

namespace gr {
namespace spectre {


batched_file_sink::sptr batched_file_sink::make(const std::string& dir,
                                                const std::string& tag,
                                                const std::string& input_type,
                                                const float batch_size,
                                                const int sample_rate,
                                                const bool group_by_date,
                                                const bool is_tagged,
                                                const std::string& tag_key,
                                                const float initial_tag_value)
{
    return gnuradio::make_block_sptr<batched_file_sink_impl>(dir,
                                                             tag,
                                                             input_type,
                                                             batch_size,
                                                             sample_rate,
                                                             group_by_date,
                                                             is_tagged,
                                                             tag_key,
                                                             initial_tag_value);
};


batched_file_sink_impl::batched_file_sink_impl(const std::string& dir,
                                               const std::string& tag,
                                               const std::string& input_type,
                                               const float batch_size,
                                               const int sample_rate,
                                               const bool group_by_date,
                                               const bool is_tagged,
                                               const std::string& tag_key,
                                               const float initial_tag_value)
    : gr::sync_block("batched_file_sink",
                     gr::io_signature::make(1, 1, get_sizeof_stream_item(input_type)),
                     gr::io_signature::make(0, 0, 0)),
      d_dir(dir),
      d_tag(tag),
      d_input_type(input_type),
      d_sizeof_stream_item(get_sizeof_stream_item(input_type)),
      d_nsamples_per_batch(get_num_samples_per_batch(batch_size, sample_rate)),
      d_is_tagged(is_tagged),
      d_group_by_date(group_by_date),
      d_nbuffered_samples(0),
      d_buffer(std::vector<char>(d_nsamples_per_batch * d_sizeof_stream_item, 0)),
      d_fdata(nullptr),
      d_ftags(nullptr),
      d_batch_time(batch_time{ std::tm{}, 0 }),
      d_buffer_state(buffer_state::EMPTY)
{
}

batched_file_sink_impl::~batched_file_sink_impl() { close_fstreams(); }

void batched_file_sink_impl::update_batch_time()
{
    using namespace std::chrono;

    // Get the current system time as a time point.
    time_point<system_clock, nanoseconds> now = system_clock::now();

    // Convert this to UTC (to seconds precision).
    std::time_t now_c = system_clock::to_time_t(now);
    std::tm* utc_now = std::gmtime(&now_c);

    // Then, extract the millisecond component.
    milliseconds ms =
        duration_cast<milliseconds>(now.time_since_epoch() - seconds(now_c));

    d_batch_time.utc_tm = *utc_now;
    d_batch_time.ms = static_cast<int>(ms.count());
}

void batched_file_sink_impl::open_fstream(std::ofstream& f, const std::string& extension)
{
    using namespace std::filesystem;

    path filename = generate_file_path(
        d_dir, d_tag, extension, d_group_by_date, d_batch_time.utc_tm, d_batch_time.ms);

    // If the parent directory does not exist, create it.
    path parent_dir{ filename.parent_path() };
    if (!exists(parent_dir)) {
        create_directories(parent_dir);
    }

    // Open the file.
    f.open(filename.string(), std::ios::binary);
    if (!f.is_open()) {
        const std::string msg = "Failed to open: " + filename.string();
        d_logger->error(msg);
        throw std::runtime_error(msg);
    }
}

void batched_file_sink_impl::open_fstreams()
{
    open_fstream(d_fdata, d_input_type);
    if (d_is_tagged) {
        open_fstream(d_ftags, "hdr");
    }
}

int batched_file_sink_impl::fill_buffer(int noutput_items, const unsigned char* in)
{
    // Fill the buffer with as many samples as possible, without exceeding its fixed size.
    // Keep a record of how many we've consumed, so we can report back to gnuradio
    // runtime.
    int n_consume = std::min(noutput_items, d_nsamples_per_batch - d_nbuffered_samples);
    std::memcpy(d_buffer.data() + d_nbuffered_samples * d_sizeof_stream_item,
                in,
                n_consume * d_sizeof_stream_item);
    d_nbuffered_samples += n_consume;
    if (d_nbuffered_samples == d_nsamples_per_batch) {
        d_buffer_state = buffer_state::FULL;
    }
    return n_consume;
}

void batched_file_sink_impl::flush_buffer()
{
    d_fdata.write(d_buffer.data(), d_sizeof_stream_item * d_nsamples_per_batch);
    d_nbuffered_samples = 0;
}


void batched_file_sink_impl::close_fstreams()
{
    if (d_fdata.is_open()) {
        d_fdata.close();
    }
    if (d_ftags) {
        d_ftags.close();
    }
}

void batched_file_sink_impl::init()
{
    // The batch is timestamped when the file is opened, so open the file now.
    update_batch_time();
    open_fstreams();
    d_buffer_state = buffer_state::FILLING;
}

void batched_file_sink_impl::reset()
{
    flush_buffer();
    close_fstreams();
    d_buffer_state = buffer_state::EMPTY;
}

int batched_file_sink_impl::work(int noutput_items,
                                 gr_vector_const_void_star& input_items,
                                 gr_vector_void_star& output_items)
{

    if (d_buffer_state == buffer_state::EMPTY) {
        init();
    }

    const unsigned char* in = reinterpret_cast<const unsigned char*>(input_items[0]);
    int n_consumed = fill_buffer(noutput_items, in);

    if (d_buffer_state == buffer_state::FULL) {
        reset();
    }

    return n_consumed;
}

} // namespace spectre
} // namespace gr
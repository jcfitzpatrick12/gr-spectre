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
#include <optional>

namespace {

static constexpr int NUM_DIGITS_MILLISECONDS = 3;
static constexpr int INPUT_PORT = 0;

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
      d_tag_key(pmt::string_to_symbol(tag_key)),
      d_initial_tag_value(initial_tag_value),
      d_batch_time(batch_time{ std::tm{}, 0 }),
      d_buffer_state(buffer_state::EMPTY),
      d_nbuffered_samples(0),
      d_data_buffer(std::vector<char>(d_nsamples_per_batch * d_sizeof_stream_item, 0)),
      d_fdata(nullptr),
      d_nbuffered_tags(0),
      // The `d_tags_buffer` is generously sized to handle the maximum possible number of
      // tags (one per sample), but the actual number of tags per batch may vary due to
      // the nature of the GNU Radio runtime. So practically, it's unlikely this will ever
      // fill entirely.
      d_tags_buffer(std::vector<float>(d_nsamples_per_batch * 2, 0)),
      d_ftags(nullptr),
      d_active_tag()
{
}

batched_file_sink_impl::~batched_file_sink_impl() { close_fstreams(); }

void batched_file_sink_impl::init()
{
    // The batch is timestamped when the file is opened, so open the file now.
    set_batch_time();
    open_fstreams();

    if (d_is_tagged) {
        set_initial_active_tag();
    }

    d_buffer_state = buffer_state::FILLING;
}

void batched_file_sink_impl::reset()
{
    flush_data_buffer();
    flush_tag_buffer();
    close_fstreams();
    d_buffer_state = buffer_state::EMPTY;
}

void batched_file_sink_impl::set_batch_time()
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

void batched_file_sink_impl::close_fstreams()
{
    if (d_fdata.is_open()) {
        d_fdata.close();
    }
    if (d_ftags) {
        d_ftags.close();
    }
}

int batched_file_sink_impl::fill_data_buffer(int noutput_items, const char* in)
{
    // Fill the buffer with as many samples as possible, without exceeding its fixed size.
    // Keep a record of how many we've consumed, so we can report back to gnuradio
    // runtime.
    int nconsumed_items =
        std::min(noutput_items, d_nsamples_per_batch - d_nbuffered_samples);
    std::memcpy(d_data_buffer.data() + d_nbuffered_samples * d_sizeof_stream_item,
                in,
                nconsumed_items * d_sizeof_stream_item);
    d_nbuffered_samples += nconsumed_items;
    if (d_nbuffered_samples == d_nsamples_per_batch) {
        d_buffer_state = buffer_state::FULL;
    }
    return nconsumed_items;
}

void batched_file_sink_impl::flush_data_buffer()
{
    // Completely flush the entire data buffer.
    d_fdata.write(d_data_buffer.data(), d_sizeof_stream_item * d_nsamples_per_batch);
    d_nbuffered_samples = 0;
}

std::optional<tag_t> batched_file_sink_impl::get_tag_from_first_sample()
{
    // Declare a vector which will store a single tag if it exists for the first sample,
    // and is empty otherwise.
    std::vector<tag_t> tags;
    uint64_t rel_start{ 0 };
    uint64_t rel_end{ 1 };
    get_tags_in_window(tags, INPUT_PORT, rel_start, rel_end, d_tag_key);
    return (tags.empty()) ? std::nullopt : std::optional<tag_t>(tags[0]);
}

bool batched_file_sink_impl::tag_is_set() const
{
    // Check if the active tag has been set by comparing it to a default-constructed tag.
    tag_t default_tag = tag_t();
    return !(d_active_tag == default_tag);
}

void batched_file_sink_impl::set_initial_active_tag()
{
    // If the first sample in the current call to work has a tag, use that.
    std::optional<tag_t> first_tag = get_tag_from_first_sample();
    if (first_tag.has_value()) {
        d_active_tag = first_tag.value();
        return;
    }

    // Otherwise, if a tag is set, all samples at the last call to work
    // have already been accounted for. So, update the offset for the current call
    // to work.
    if (tag_is_set()) {
        d_active_tag.offset = nitems_read(INPUT_PORT);
        return;
    }

    // As a fallback, use the user-defined tag value to initialise the active tag.
    if (d_initial_tag_value) {
        // Use zero here, as the tag is attached to the first in the stream.
        d_active_tag.offset = 0;
        d_active_tag.key = d_tag_key;
        d_active_tag.value = pmt::from_float(d_initial_tag_value);
        d_active_tag.srcid = pmt::intern(alias());
        return;
    }

    // If none of the above conditions are met, throw an error.
    throw std::runtime_error("Undefined tag state. Ensure the first sample is tagged or "
                             "provide an initial value.");
}

void batched_file_sink_impl::fill_tag_buffer(int nconsumed_items)
{
    // Get all the tags ranging from the sample with the active tag,
    // to however many items have been consumed by the current call to work.
    std::vector<tag_t> tags;
    uint64_t abs_start = d_active_tag.offset + 1;
    uint64_t abs_end = nitems_read(INPUT_PORT) + nconsumed_items;
    get_tags_in_range(tags, INPUT_PORT, abs_start, abs_end);
    size_t num_tags = tags.size();

    for (size_t n = 0; n < num_tags; n++) {
        // Compute the number of samples associated with each tag by comparing the offset
        // of the current tag to the offset of the next tag.
        tag_t next_tag{ tags[n] };
        float tag_value = pmt::to_float(d_active_tag.value);
        float num_samples = static_cast<float>(next_tag.offset - d_active_tag.offset);

        // Record the tag value, along with the number of samples at that value.
        d_tags_buffer[2 * d_nbuffered_tags] = tag_value;
        d_tags_buffer[2 * d_nbuffered_tags + 1] = num_samples;
        d_nbuffered_tags++;

        // Finally, update the active tag.
        d_active_tag = next_tag;
    }

    if (d_buffer_state == buffer_state::FULL) {
        // At this point, we want to close the file. however, we've yet to record how many
        // samples are associated with the current (last available) active tag, since
        // there's no "next tag" to compute it exactly using offsets. Instead, we simply
        // compute the number of samples remaining.
        float tag_value = pmt::to_float(d_active_tag.value);
        float num_samples_remaining = static_cast<float>(abs_end - d_active_tag.offset);
        d_tags_buffer[2 * d_nbuffered_tags] = tag_value;
        d_tags_buffer[2 * d_nbuffered_tags + 1] = num_samples_remaining;
        d_nbuffered_tags++;
    }
}

void batched_file_sink_impl::flush_tag_buffer()
{
    const char* s = reinterpret_cast<const char*>(d_tags_buffer.data());
    // For each tag buffered, we recorded the tag value and the number of samples
    // associated with that value, both as single precision floats.
    size_t num_chars = 2 * d_nbuffered_tags * sizeof(float);
    d_ftags.write(s, num_chars);
    d_nbuffered_tags = 0;
};


int batched_file_sink_impl::work(int noutput_items,
                                 gr_vector_const_void_star& input_items,
                                 gr_vector_void_star& output_items)
{

    if (d_buffer_state == buffer_state::EMPTY) {
        init();
    }

    const char* in = reinterpret_cast<const char*>(input_items[0]);
    int nconsumed_items = fill_data_buffer(noutput_items, in);

    if (d_is_tagged) {
        // Only record tags up to the number of consumed items in the input buffer.
        fill_tag_buffer(nconsumed_items);
    }


    if (d_buffer_state == buffer_state::FULL) {
        reset();
    }

    return nconsumed_items;
}

} // namespace spectre
} // namespace gr
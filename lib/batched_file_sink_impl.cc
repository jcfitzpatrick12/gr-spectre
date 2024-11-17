/* -*- c++ -*- */
/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "batched_file_sink_impl.h"
#include <gnuradio/io_signature.h>
#include <filesystem>
#include <fstream>

namespace gr {
namespace spectre {

using input_type = gr_complex;
batched_file_sink::sptr batched_file_sink::make(const std::string parent_dir_path,
                                                const std::string tag,
                                                const float batch_size,
                                                const int samp_rate,
                                                const bool is_sweeping,
                                                const std::string frequency_tag_key,
                                                const float initial_center_frequency)
{
    return gnuradio::make_block_sptr<batched_file_sink_impl>(parent_dir_path,
                                                             tag,
                                                             batch_size,
                                                             samp_rate,
                                                             is_sweeping,
                                                             frequency_tag_key,
                                                             initial_center_frequency);
}


// Private constructor.
batched_file_sink_impl::batched_file_sink_impl(const std::string parent_dir_path,
                                               const std::string tag,
                                               const float batch_size,
                                               const int samp_rate,
                                               const bool is_sweeping,
                                               const std::string frequency_tag_key,
                                               const float initial_center_frequency)
    : gr::sync_block("batched_file_sink",
                     gr::io_signature::make(
                         1 /* min inputs */, 1 /* max inputs */, sizeof(input_type)),
                     gr::io_signature::make(0, 0, 0)),
      _parent_dir_path(parent_dir_path),
      _tag(tag),
      _sampling_interval( 1.0f / samp_rate ),
      _samples_per_batch(batch_size / _sampling_interval),
      _is_sweeping(is_sweeping),
      _frequency_tag_key(pmt::string_to_symbol(frequency_tag_key)),
      _initial_center_frequency(initial_center_frequency),
      _create_new_batch(true),
      _active_tag(), 
      _default_tag(),
      _input_port(0)
{ 

}


// Virtual destructor.
batched_file_sink_impl::~batched_file_sink_impl() 
{
    _bin_file.close();
    _hdr_file.close();
}


std::string batched_file_sink_impl::get_file_extension(batch_file_type file_type)
{
    switch (file_type) 
    {
        case batch_file_type::BIN: return ".bin";
        case batch_file_type::HDR: return ".hdr";
        default: throw std::invalid_argument("Unexpected batch file type!");
    }
}


batch_time batched_file_sink_impl::get_batch_time()
{
    using namespace std::chrono;

    // Capture the current system time as a time point.
    auto now = system_clock::now();

    // Compute the time elapsed since the Unix epoch (00:00:00 1st Jan 1970 UTC)
    time_t now_c = system_clock::to_time_t(now);
    // Copy the `std::tm` struct since `std::gmtime` returns a pointer to static memory.
    std::tm* utc_datetime = std::gmtime(&now_c);

    // Compute the millisecond fractional component of the current system time.
    auto time_since_unix_epoch = now.time_since_epoch();
    auto seconds_since_unix_epoch = seconds(static_cast<int64_t>(now_c));
    auto chrono_millisecond_component = duration_cast<milliseconds>(time_since_unix_epoch - seconds_since_unix_epoch);
    int64_t millisecond_component { chrono_millisecond_component.count() };

    // Return the `batch_time` object with a copy of `std::tm`.
    return batch_time{utc_datetime, millisecond_component};
}


fs::path batched_file_sink_impl::get_absolute_batch_file_path(std::tm* utc_datetime, 
                                                              batch_file_type file_type) const
{    
    std::ostringstream oss;
    oss << std::put_time(utc_datetime, "%Y/%m/%d/") 
        << std::put_time(utc_datetime, "%Y-%m-%dT%H:%M:%S")
        << "_"
        << _tag
        << get_file_extension(file_type);
    return _parent_dir_path / fs::path(oss.str());    
}


batch_metadata batched_file_sink_impl::get_new_batch_metadata() const
{
    batch_time current_batch_time { get_batch_time() };
    fs::path bin_file_path { get_absolute_batch_file_path(current_batch_time.utc_datetime, batch_file_type::BIN ) };
    fs::path hdr_file_path { get_absolute_batch_file_path(current_batch_time.utc_datetime, batch_file_type::HDR ) };
    return batch_metadata {current_batch_time, bin_file_path, hdr_file_path};
}


void batched_file_sink_impl::open_batch_file(std::ofstream& file,
                                             fs::path file_path)
{
    // Create the parent path if it does not already exist.
    fs::path dated_parent_directory { file_path.parent_path() };
    if (!fs::exists(dated_parent_directory))
    {
        fs::create_directories(dated_parent_directory);
    }
    // If the file is open, close it before opening another file.
    if (file.is_open())
    {
        file.close();
    }

    // Open a new file at the appropriate file path.
    file.open(file_path, std::ios::binary);
}


void batched_file_sink_impl::write_millisecond_component_to_detached_header()
{
    // Cast as a float for consistent reading back in SPECTRE.
    float millisecond_component { static_cast<float>(_batch_metadata.time.millisecond_component) };
    write_to_file(_hdr_file, &millisecond_component, 1);
}


bool batched_file_sink_impl::tag_is_set(tag_t tag) const
{ 
    return !( tag == _default_tag);
}


std::optional<tag_t> batched_file_sink_impl::get_tag_from_first_sample()
{   
    // "tags" is a vector which will store a single tag if it exists for the first sample,
    // and is empty otherwise.
    std::vector<tag_t> tags;
    uint64_t rel_start { 0 };
    uint64_t rel_end { 1 };
    get_tags_in_window(tags, 
                       _input_port, 
                       rel_start, rel_end, 
                       _frequency_tag_key);

    if (tags.empty())
    {
        return std::nullopt;
    }
    else
    {
        return tags[0];
    }
}


bool batched_file_sink_impl::initial_center_frequency_is_set() const
{
    return (_initial_center_frequency != 0);
}


void batched_file_sink_impl::set_initial_active_tag()
{
    // If the first sample has a tag, use that!
    std::optional<tag_t> first_tag = get_tag_from_first_sample();
    if ( first_tag )
    {
        _active_tag = *first_tag;
        return;
    }

    // If a tag is already set, update the offset for the current work function call.
    if ( tag_is_set(_active_tag) ) 
    {
        _active_tag.offset = nitems_read(0);
        return;
    }

    // If no tag is set, use the user-defined initial center frequency to define the active tag.
    // This should only ever be reached at the first call of the work function.
    if ( initial_center_frequency_is_set() )
    {
        // Use zero here, as the tag is attached to the first in the stream.
        _active_tag.offset = 0;
        _active_tag.key = _frequency_tag_key;
        _active_tag.value = pmt::from_float(_initial_center_frequency);
        _active_tag.srcid = pmt::intern(alias());
        return;
    }

    // If none of the above conditions are met, throw an error.
    throw std::runtime_error(
        "Undefined tag state: Unable to infer center frequency of the first sample. "
        "Verify input tags or set an initial center frequency."
    );
}

std::vector<float> batched_file_sink_impl::get_num_samples_per_center_frequency(int noutput_items)
{
    // Get all frequency tags ranging from the current active tag (in absolute item time)
    // to the end of the current call to work (in absolute item time)
    std::vector<tag_t> tags;
    uint64_t abs_start { _active_tag.offset + 1 };
    uint64_t abs_end { nitems_read(_input_port) + noutput_items };
    get_tags_in_range(tags, 
                      _input_port, 
                      abs_start, abs_end, 
                      _frequency_tag_key);
 
    // Create a vector which will hold the number of samples per center frequency
    // That is, effectively, an ordered list of pairs (freq_i, num_samples_at_freq_i)
    // All numerical values are cast as floats for consistent reading back in SPECTRE.
    uint64_t num_tags { tags.size() };
    uint64_t num_elements = ( 2 * num_tags );
    std::vector<float> sweep_metadata(num_elements, 0.0f);

    for ( uint64_t n {0}; n < num_tags; n++)
    {
        tag_t next_tag { tags[n] };
        // Infer the number of samples at each center frequency through the tag offsets.
        uint64_t num_samples { next_tag.offset - _active_tag.offset };
        float active_frequency_f { pmt::to_float(_active_tag.value) };
        float num_samples_f { static_cast<float>(num_samples) };
        // Copy the values into the array.
        sweep_metadata[ 2*n ] = active_frequency_f;
        sweep_metadata[ 2*n + 1 ] = num_samples_f;
        // Update the active tag
        _active_tag = next_tag;
    }

    if (_create_new_batch)
    {
        // Essentially, at this moment we have handled all the tags in the current call of the work function. 
        // Thus, there is no "next tag" to evaluate the total number of samples associated with the current active tag. 
        // For a clean handover to the next file, we compute the number of samples remaining at the active tag in the current call of the work function.
        // In this way, we will have declared for every IQ sample dumped to the bin file, what frequency it was collected at.

        uint64_t num_samples_remaining = abs_end - _active_tag.offset;
        float active_frequency_f { pmt::to_float(_active_tag.value) };
        float num_samples_remaining_f = static_cast<float>(num_samples_remaining);
        sweep_metadata.push_back(active_frequency_f);
        sweep_metadata.push_back(num_samples_remaining_f);
    }

    return sweep_metadata;
}


void batched_file_sink_impl::initialise_new_batch() 
{
    // Effectively reset the elapsed time for the current batch.
    _sample_count = 0;
    // Reset boolean flag indicating new batch will be created.
    _create_new_batch = false;
    // Update the batch metadata member variable with the current system time.
    _batch_metadata = get_new_batch_metadata();

    open_batch_file(_bin_file, _batch_metadata.bin_file_path);
    open_batch_file(_hdr_file, _batch_metadata.hdr_file_path);
    write_millisecond_component_to_detached_header();

    if (_is_sweeping)
    {
        set_initial_active_tag();
    }
}


int batched_file_sink_impl::work(int noutput_items,
                                 gr_vector_const_void_star& input_items,
                                 gr_vector_void_star& output_items)
{   
    if (_create_new_batch)
    {
        initialise_new_batch();
    }
    
    // Write the input samples as contiguous raw bytes to the binary file.
    const input_type* in = static_cast<const input_type*>(input_items[0]);
    write_to_file(_bin_file, in, noutput_items);

    // Update the elapsed time (inferred by sample counting).
    _sample_count += noutput_items;

    // If the elapsed time exceeds the batch size...
    if (_sample_count >= _samples_per_batch)
    {
        // At the next call of the work function, create a new batch.
        _create_new_batch = true;
    }

    if (_is_sweeping)
    {
        std::vector<float> sweep_metadata { get_num_samples_per_center_frequency(noutput_items) };
        write_to_file(_hdr_file, sweep_metadata.data(), sweep_metadata.size());

    }

    return noutput_items;
}


} /* namespace spectre */
} /* namespace gr */

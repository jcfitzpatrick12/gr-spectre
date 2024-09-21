#include "gnuradio/spectre/bin_chunk_helper.h"

namespace gr 
{
namespace spectre
{

bin_chunk_helper::bin_chunk_helper(
    std::string parent_path,
    std::string tag
){
    _parent_path = fs::path(parent_path); // convert the user inputted string parent path to a path object
    _tag = tag; // set as is with the user inputted string
};

bin_chunk_helper::~bin_chunk_helper(){};

void bin_chunk_helper::update() {
    // Get current system time
    auto now = std::chrono::system_clock::now();
    
    // Extract seconds and milliseconds since epoch
    auto seconds_since_epoch = std::chrono::time_point_cast<std::chrono::seconds>(now);
    auto ms_remainder = std::chrono::duration_cast<std::chrono::milliseconds>(now - seconds_since_epoch);
    
    // Convert the time_point to std::time_t for formatting
    std::time_t current_time = std::chrono::system_clock::to_time_t(seconds_since_epoch);
    
    // Convert std::time_t to a tm struct for formatting
    std::tm* current_gmt = std::gmtime(&current_time);

    // Create the date path and file names using stringstreams
    std::stringstream date_path_ss, bin_chunk_name_ss, hdr_chunk_name_ss;
    date_path_ss << std::put_time(current_gmt, "%Y/%m/%d");
    bin_chunk_name_ss << std::put_time(current_gmt, "%Y-%m-%dT%H:%M:%S") << "_" << _tag << ".bin";
    hdr_chunk_name_ss << std::put_time(current_gmt, "%Y-%m-%dT%H:%M:%S") << "_" << _tag << ".hdr";

    // Convert the formatted strings into paths
    fs::path date_path = fs::path(date_path_ss.str());
    fs::path bin_chunk_name = fs::path(bin_chunk_name_ss.str());
    fs::path hdr_chunk_name = fs::path(hdr_chunk_name_ss.str());

    // Set the appropriate member variables
    _bin_absolute_path = _parent_path / date_path / bin_chunk_name;
    _hdr_absolute_path = _parent_path / date_path / hdr_chunk_name;
    _parent_path_with_date_dirs = _parent_path / date_path;
    _millisecond_correction = static_cast<int32_t>(ms_remainder.count());
}


fs::path bin_chunk_helper::get_bin_absolute_path(){
    return _bin_absolute_path;
};


fs::path bin_chunk_helper::get_hdr_absolute_path(){
    return _hdr_absolute_path;
}


fs::path bin_chunk_helper::get_parent_path_with_date_dirs(){
    return _parent_path_with_date_dirs;
};


int bin_chunk_helper::get_millisecond_correction(){
    return _millisecond_correction;
}


};
};


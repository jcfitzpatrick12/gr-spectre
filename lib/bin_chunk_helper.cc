#include "gnuradio/spectre/bin_chunk_helper.h"

namespace gr 
{
namespace spectre
{

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


int bin_chunk_helper::get_millisecond_correction(){
    return _millisecond_correction;
}


};
};


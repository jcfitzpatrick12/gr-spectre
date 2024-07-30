#ifndef INCLUDED_BIN_CHUNK_UTILS_H
#define INCLUDED_BUN_CHUNK_UTILS_H

#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <ctime>

namespace fs = std::filesystem;

namespace gr 
{
namespace spectre
{

class bin_chunk_helper
{
private:
    fs::path _parent_path; // Passed in at instantiation. Defines the absolute path to the parent directory to hold the batched files. 
    std::string _tag; // Passed in at instantiation. File names are tagged with this string. 
    fs::path _bin_absolute_path; // Computed when calling update, based on system time. The absolute path to the binary chunk file. 
    fs::path _hdr_absolute_path; // Computed when calling update, based on system time. The absolute path to the header chunk file. 
    fs::path _parent_path_with_date_dirs; // Computed when calling update, based on system time. The absolute parent path for the bin and header files. 
    int32_t _millisecond_correction; // Computed when calling update, based on system time. A 32-bit integer describing the millisecond correction for the timestamp of the first sample in each batch.

public:
    bin_chunk_helper(
        std::string parent_path,
        std::string tag
    ); // Constructor
    ~bin_chunk_helper(); // Destructor
    void update(); // Effectively a setter for the _millisecond_correction, _bin_absolute_path and _hdr_absolute_path member variables.
    fs::path get_bin_absolute_path(); // Public getter for the _bin_absolute_path member variable. 
    fs::path get_hdr_absolute_path(); // Public getter for the _hdr_absolute_path member variable. 
    fs::path get_parent_path_with_date_dirs(); // Public getter for the parent path of the binary/header chunk files.
    int32_t get_millisecond_correction(); // Public getter for the _millisecond_correction member variable. 
};
};
};
#endif
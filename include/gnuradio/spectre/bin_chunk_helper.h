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
    fs::path _parent_path; /**< Defines the absolute path to the parent directory to hold the batched files. */
    std::string _tag; /**< File names are tagged with this string. */
    int32_t _millisecond_correction; /**< A 32-bit integer describing the millisecond correction for the timestamp of the first sample in each batch.*/
    fs::path _absolute_path; /**< The absolute path to the binary chunk file. */

public:
    bin_chunk_helper(
        std::string parent_path,
        std::string tag
    );

    ~bin_chunk_helper();
    void set_attrs(); /**< Effectively a setter for the _millisecond_correction and _absolute_path member variables. */
    fs::path get_absolute_path(); /**< Public getter for the _absolute_path member variable. */
    int32_t get_millisecond_correction(); /**< Public getter for the _millisecond_correction member variable. */
    fs::path get_absolute_parent_path(); /**< Public getter for the parent path of the binary chunk file.*/
};
};
};
#endif
#include "utils.h"

#include <gnuradio/types.h>

namespace gr {
namespace spectre {

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

} // namespace spectre
} // namespace gr
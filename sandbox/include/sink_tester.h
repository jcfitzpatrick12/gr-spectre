#ifndef SINK_TESTER_H
#define SINK_TESTER_H

#include <gnuradio/sync_block.h>

#include "base_periodic_signal.h"
#include "sink_tester.h"
#include "periodic_signals.h"

// Test a sink block with repeated calls to the work function.
// Assumes the sink has a single input port.
// Assumes the input stream has type std::complex<float>.
// Input is identical at each call of the work function.
// Input and output items are of fixed size.
namespace sandbox 
{

class sink_tester
{
public:
    // Constructor.
    sink_tester(const std::shared_ptr<gr::sync_block> sink,
                std::unique_ptr<base_periodic_signal> source_signal);

    // Destructor.
    ~sink_tester();

    // Call the work function an arbitrary number of times
    void call_work(const int num_calls);
    
private:
    // Shared pointer to the sink block.
    const std::shared_ptr<gr::sync_block> _sink;
    std::unique_ptr<base_periodic_signal> _source_signal;
    
    // Input buffer and pointer to the buffer.
    gr_vector_complex _input_samples;
    gr_vector_const_void_star _input_items;

    // Output buffer and pointer to the buffer.
    gr_vector_complex _output_samples;
    gr_vector_void_star _output_items;
};

}

#endif
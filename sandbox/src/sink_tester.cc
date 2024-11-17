#include <iostream>

#include <gnuradio/sync_block.h>

#include "base_periodic_signal.h"
#include "sink_tester.h"
#include "periodic_signals.h"

namespace sandbox 
{

// Constructor
sink_tester::sink_tester(const std::shared_ptr<gr::sync_block> sink,
                         std::unique_ptr<base_periodic_signal> source_signal) 
    : _sink(sink),
      _source_signal(std::move(source_signal)),
      _input_samples(_source_signal->get_samples()),
      _input_items{_input_samples.data()},
      _output_samples(_source_signal->get_num_samples(), gr_complex(0, 0)),
      _output_items{_output_samples.data()}
{
}

// Destructor.
sink_tester::~sink_tester(){}


// Invoke the work function repeatedly.
void sink_tester::call_work(const int num_calls)
{
    for (int n {0}; n < num_calls; n++ )
    {   
        // Call the work function on each iteration.
        _sink->work(_source_signal->get_num_samples(), 
                    _input_items,
                    _output_items);
    }
}

}
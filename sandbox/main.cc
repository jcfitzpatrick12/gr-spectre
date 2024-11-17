#include <iostream>

#include "types.h"
#include "sink_tester.h"
#include "base_periodic_signal.h"
#include "periodic_signals.h"
#include "gnuradio/spectre/batched_file_sink.h"


using namespace sandbox;

int main() 
{
    // Define the source signal.
    periodic_signal_type source_signal_type { periodic_signal_type::SIN_WAVE };
    int num_samples { 8000 };

    // Constructor arguments for batched file sink
    const std::string parent_dir_path = "./chunks/";
    const std::string tag = "my-tag";
    const float batch_size = 1.0;
    const int samp_rate = 32000;
    const bool is_sweeping = false;
    const std::string frequency_tag_key = "freq";
    const float initial_center_frequency = 0;

    // How many times will we call the sink?
    const int num_calls { 1 };

    // Use our factory function to fetch an instance of signal_source.
    std::unique_ptr<base_periodic_signal> signal_source { get_periodic_signal(source_signal_type, 
                                                                              num_samples,
                                                                              samp_rate) };


    // Use the public interface to create an instance of batched file sink.
    std::shared_ptr<gr::spectre::batched_file_sink> sink { gr::spectre::batched_file_sink::make(parent_dir_path,
                                                                                                tag,
                                                                                                batch_size,
                                                                                                samp_rate,
                                                                                                is_sweeping,
                                                                                                frequency_tag_key,
                                                                                                initial_center_frequency) };

    // Test batched file sink.
    // Transfer ownership of the signal source to the instance of sink_tester.
    sink_tester tester(sink, std::move(signal_source));
    tester.call_work(num_calls);
    
    return 0;
}


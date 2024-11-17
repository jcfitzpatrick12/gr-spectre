#include <cmath>

#include "sin_wave.h"
#include "types.h"

namespace sandbox 
{

sin_wave::sin_wave(const int num_samples,
                   const int samp_rate) 
        : base_periodic_signal(num_samples, samp_rate)
{
    initialise_samples();
}

const gr_vector_complex sin_wave::compute_samples() const
{
    // Initialise a vector that to hold the complex samples
    gr_vector_complex signal(_num_samples, gr_complex(0, 0));

    // Populate according to a sin wave periodic with period T
    for (int n {0}; n < _num_samples; ++n)
    {
        gr_complex z { 2 * M_PIf * get_time(n) / get_period() };
        signal[n] = std::sin(z);
    }
    return signal;
}

}
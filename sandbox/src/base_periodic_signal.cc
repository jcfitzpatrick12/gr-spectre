#include <vector>
#include <complex>

#include "base_periodic_signal.h"
#include "types.h"
#include "constants.h"

namespace sandbox 
{

base_periodic_signal::base_periodic_signal(const int num_samples,
                                           const int samp_rate)
                   : _num_samples(num_samples),
                     _samp_rate(samp_rate),
                     _times( compute_times() ),
                     _samples( gr_vector_complex(_num_samples, 0.0) )
                     {}

base_periodic_signal::~base_periodic_signal() {}


const int base_periodic_signal::get_num_samples() const
{
    return _num_samples;
}


const int base_periodic_signal::get_samp_rate() const
{
    return _samp_rate;
}


const float base_periodic_signal::get_period() const
{
    return (_num_samples) * (1/static_cast<float>(_samp_rate));
}


const float base_periodic_signal::get_time(const int sample_index) const
{
    return sample_index * (1/static_cast<float>(_samp_rate));
}


const gr_vector_float& base_periodic_signal::get_times() const
{
    return _times;
}


const gr_vector_complex& base_periodic_signal::get_samples() const
{
    return _samples;
}


const gr_vector_const_void_star base_periodic_signal::get_samples_ptr() const
{
    return gr_vector_const_void_star(1, static_cast<const void*>(_samples.data()));
}



const gr_vector_float base_periodic_signal::compute_times()
{

    gr_vector_float times(_num_samples, 0 );

    for (int k {0}; k < _num_samples; ++k)
    {
        times[k] = get_time(k);
    }

    return times;
};


void base_periodic_signal::initialise_samples() 
{
    _samples = compute_samples();
}


}
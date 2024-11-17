#ifndef SIN_WAVE_H
#define SIN_WAVE_H

#include "base_periodic_signal.h"

namespace sandbox 
{

class sin_wave : public base_periodic_signal
{
public:
    sin_wave(const int num_samples,
             const int samp_rate);
    const gr_vector_complex compute_samples() const override;
};

}

#endif
#ifndef PERIODIC_SIGNALS_H
#define PERIODIC_SIGNALS_H

#include "base_periodic_signal.h"

namespace sandbox 
{

enum class periodic_signal_type 
{
    SIN_WAVE
};


std::unique_ptr<base_periodic_signal> get_periodic_signal(periodic_signal_type signal_type,
                                                          const int num_samples,
                                                          const int samp_rate);

}

#endif
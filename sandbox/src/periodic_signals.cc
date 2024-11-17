#include "base_periodic_signal.h"
#include "periodic_signals.h"
#include "sin_wave.h"

namespace sandbox
{

std::unique_ptr<base_periodic_signal> get_periodic_signal(periodic_signal_type signal_type,
                                                          const int num_samples,
                                                          const int samp_rate)
{
    switch (signal_type)
    {
        case periodic_signal_type::SIN_WAVE:
            return std::make_unique<sin_wave>(num_samples, samp_rate);
        default:
            throw std::invalid_argument("Periodic signal type unrecognised!");
    }
}
}
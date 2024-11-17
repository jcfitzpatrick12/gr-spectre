#ifndef BASE_PERIODIC_SIGNAL_H
#define BASE_PERIODIC_SIGNAL_H

#include "types.h"

namespace sandbox 
{

// Assume equidistant samples have been taken over one period.
// Samples are computed by derived classes.
class base_periodic_signal
{
public:

    // Base class constructor.
    base_periodic_signal(const int num_samples,
                         const int samp_rate);

    // Base class destructor.
    virtual ~base_periodic_signal();
    
    // Get the total number of samples taken over the period.
    const int get_num_samples() const;

    // Get the sample rate.
    const int get_samp_rate() const;

    // Compute the period of the signal.
    // Assumes the signal repeats after the samples provided.
    const float get_period() const;

    // Get the physical time associated with the index of a sample.
    const float get_time(const int sample_index) const;

    // Get a vector of the physical times associated with each sample
    const gr_vector_float& get_times() const;

    // Get a vector of the complex periodic signal samples.
    const gr_vector_complex& get_samples() const;

    // Get a pointer to the underlying samples array
    const gr_vector_const_void_star get_samples_ptr() const;


protected:
    // The total number of samples
    const int _num_samples;
    // The (imposed) sampling rate.
    const int _samp_rate;
    // The physical times associated with each sample.
    const gr_vector_float _times;
    // The values of each sample.
    gr_vector_complex _samples;

    // Compute an array assigning physical times to each sample.
    // Assumes samples are taken equidistantly over the period.
    const gr_vector_float compute_times();

    // Compute each of the complex sample values.
    // Pure virtual member function must be implemented by a derived class.
    virtual const gr_vector_complex compute_samples() const = 0;

    // Used to set the signal member variable in the derived classes constructor.
    void initialise_samples();

};

}
#endif

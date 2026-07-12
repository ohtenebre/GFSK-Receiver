#include "dsp.hpp"
#include "mmse_interp_taps.hpp"

#include <cmath>

// Fractional resampling using MMSE interpolation
std::vector<std::complex<float>> FractionalResampler::process(const std::vector<std::complex<float>> &input)
{
    std::vector<std::complex<float>> output;

    if (input.size() < NTAPS)
        return output;

    int ii = NTAPS - 1;

    while (ii < (int)input.size() - 7)
    {
        output.push_back(interpolate(&input[ii], d_mu));

        double s = d_mu + d_ratio;

        double f = floor(s);

        int incr = (int)f;

        d_mu = s - f;

        ii += incr;
    }

    return output;
}

// MMSE FIR interpolation at fractional delay mu
std::complex<float> FractionalResampler::interpolate(const std::complex<float> *input, double mu)
{
    int imu = (int)(mu * NSTEPS);

    if (imu < 0)
        imu = 0;

    if (imu > NSTEPS)
        imu = NSTEPS;

    const float *t = dsp_detail::taps[imu];

    std::complex<float> acc = input[0] * t[0];

    for (int i = 1; i < NTAPS; i++)
    {
        acc += input[-i] * t[i];
    }

    return acc;
}

// Frequency translation by complex rotation
void FrequencyShifter::process(std::vector<std::complex<float>> &IQ, uint32_t sample_rate, float freq_offset)
{
    if (IQ.empty() || sample_rate == 0 || freq_offset == 0.0f)
        return;

    const float phase_step = 2.0f * M_PIf * freq_offset / sample_rate;

    for (size_t t = 0; t < IQ.size(); ++t)
    {
        float sin_val, cos_val;
        sincosf(current_phase, &sin_val, &cos_val);
        std::complex<float> rotator(cos_val, sin_val);

        IQ[t] *= rotator;

        current_phase += phase_step;

        if (current_phase > M_PIf)
            current_phase -= 2.0f * M_PIf;
        if (current_phase < -M_PIf)
            current_phase += 2.0f * M_PIf;
    }
}

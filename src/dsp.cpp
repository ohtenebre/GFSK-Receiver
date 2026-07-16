#include "dsp.hpp"
#include "mmse_interp_taps.hpp"

#include <cmath>
#include <cstddef>
#include <vector>

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

// Generate low-pass FIR filter coefficients
std::vector<float> generate_lowpass(float gain, float sample_rate, float cutoff, float transition)
{
    // Estimate number of taps from transition width
    int ntaps = 5 * static_cast<int>(sample_rate / transition);

    if (!(ntaps & 1))
        ntaps++;

    std::vector<float> taps(ntaps);

    float fc = cutoff / sample_rate;
    int M = ntaps - 1;

    float sum = 0.f;

    for (int n = 0; n < ntaps; ++n)
    {
        float m = n - M / 2.f;

        float sinc;

        // Ideal low-pass impulse response
        if (fabs(m) < 1e-6)
            sinc = 2.f * fc;
        else
            sinc = sinf(2.f * M_PI * fc * m) / (M_PI * m);

        // Blackman window
        float w = 0.42f - 0.5f * cosf(2.f * M_PI * n / M) + 0.08f * cosf(4.f * M_PI * n / M);

        taps[n] = sinc * w;
        sum += taps[n];
    }

    // Normalize filter gain
    for (auto &t : taps)
        t = t * gain / sum;

    return taps;
}

// Apply FIR filter to input samples
std::vector<float> FIRFilter::process(const std::vector<float> &input)
{
    std::vector<float> out(input.size());

    for (size_t n = 0; n < input.size(); ++n)
    {
        // Shift delay line
        for (size_t i = delay.size() - 1; i > 0; i--)
            delay[i] = delay[i - 1];

        delay[0] = input[n];

        float acc = 0;

        // FIR convolution
        for (size_t i = 0; i < taps.size(); ++i)
            acc += delay[i] * taps[i];

        out[n] = acc;
    }

    return out;
}

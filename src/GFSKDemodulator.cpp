#include "GFSKDemodulator.hpp"
#include "mmse_fir_interpolator.hpp"

#include <cmath>
#include <cstddef>
#include <vector>

// Fast approximation of atan2()
inline float fast_atan2f(float y, float x)
{
    constexpr float PI = 3.14159265f;
    constexpr float PIBY2 = 1.5707963f;

    if (x == 0.0f && y == 0.0f)
        return 0.0f;

    float ax = fabsf(x), ay = fabsf(y);
    float mn = std::min(ax, ay);
    float mx = std::max(ax, ay);

    float a = mn / mx;
    float s = a * a;

    // Polynomial approximation
    float r = ((-0.0464964749f * s + 0.15931422f) * s - 0.327622764f) * s * a + a;

    if (ay > ax)
        r = PIBY2 - r;
    if (x < 0.0f)
        r = PI - r;
    if (y < 0.0f)
        r = -r;

    return r;
}

// Quadrature demodulator (FM discriminator)
std::vector<float> QuadratureDemod::process(const std::vector<std::complex<float>> &iq)
{
    std::vector<float> output(iq.size(), 0);
    for (size_t i = 0; i < iq.size(); ++i)
    {
        // Save first sample for phase difference calculation
        if (d_first)
        {
            d_last_sample = iq[i];
            output[i] = 0.f;
            d_first = false;

            continue;
        }

        // Phase difference between adjacent samples
        std::complex<float> product = iq[i] * std::conj(d_last_sample);

        // Extract instantaneous frequency
        float phase = fast_atan2f(product.imag(), product.real());

        // Apply demodulator gain
        output[i] = phase * d_gain;

        d_last_sample = iq[i];
    }

    return output;
}

// Symbol timing recovery using Gardner TED
SymbolSync::SymbolSync(float sps, float loop_bw, float damping, float ted_gain, float max_dev)
{
    d_sps = sps;
    d_omega = sps;
    d_mu = 0;
    d_last_sample = 0.f;

    float alpha = damping * loop_bw * 2.0f;
    float beta = loop_bw * loop_bw;

    d_gain_mu = alpha / ted_gain;

    d_gain_omega = beta / ted_gain;

    d_max_deviation = max_dev;
}

std::vector<float> SymbolSync::process(const std::vector<float> &input)
{
    std::vector<float> output;

    // Current position in input stream
    float index = 8.f;

    while (index + 8 < input.size())
    {
        int i = static_cast<int>(index);
        // Interpolate sample at current timing phase
        float sample = dsp_detail::mmse_interpolate(&input[i], d_mu);

        // Position half symbol before current sample
        float mid_index_f = index - d_omega / 2.0f;

        float mid = 0.f;
        if (mid_index_f >= 0.0f)
        {
            int mid_i = (int)mid_index_f;
            float mid_mu = mid_index_f - mid_i;
            // Half-symbol interpolated sample
            mid = dsp_detail::mmse_interpolate(&input[mid_i], mid_mu);
        }

        // Output synchronized symbol
        output.push_back(sample);

        // Gardner timing error detector
        float error = (sample - d_last_sample) * mid;

        // Correct samples-per-symbol estimate
        d_omega += d_gain_omega * error;

        // Limit clock drift
        if (d_omega > d_sps + d_max_deviation)
            d_omega = d_sps + d_max_deviation;
        if (d_omega < d_sps - d_max_deviation)
            d_omega = d_sps - d_max_deviation;

        // Update interpolation phase
        d_mu += d_omega + d_gain_mu * error;

        // Move input pointer when full sample consumed
        while (d_mu >= 1.0f)
        {
            d_mu -= 1.0f;
            index += 1.0f;
        }

        d_last_sample = sample;
    }

    return output;
}

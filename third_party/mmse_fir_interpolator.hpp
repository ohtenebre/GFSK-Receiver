#pragma once
#include "mmse_interp_taps.hpp"

#include <algorithm>
#include <stdexcept>

/*
 * Uses coefficient table from GNU Radio.
 * See third_party/gnuradio/mmse_interp_taps.hpp
*/

namespace dsp_detail
{
    template <typename sample_t>
    inline sample_t mmse_interpolate(const sample_t *input, float mu)
    {
        mu = std::clamp(mu, 0.0f, 1.0f);
        int imu = static_cast<int>(mu * NSTEPS);

        if (imu < 0 || imu > NSTEPS)
            throw std::runtime_error("mmse_interpolate: mu out of bounds");

        const float *t = taps[imu];

        sample_t acc = t[0] * input[NTAPS - 1];
        for (int k = 1; k < NTAPS; ++k)
        {
            acc += t[k] * input[NTAPS - 1 - k];
        }

        return acc;
    }

} // namespace dsp_detail

#pragma once

#include <complex>
#include <cstdint>
#include <vector>

// GFSK demodulator with quadrature detection and Mueller-Muller clock recovery
class GFSKDemodulator {
  private:
    // Previous IQ sample for phase detection
    std::complex<float> last_sample;

    // Quadrature demodulator gain
    float d_gain;

    // Buffer for timing interpolation
    std::vector<float> d_interp_buf;

    // Fractional sampling position
    float mu;

    // Symbol timing loop parameters
    float d_avg_period;
    float d_inst_period;

    float d_min_period;
    float d_max_period;
    float d_nom_period;

    // Loop filter coefficients
    float d_alpha;
    float d_beta;

    // Previous symbols for timing error detector
    float last_rx_symbol;
    float last_sliced_symbol;

    // Samples per symbol
    float d_samples_per_symbol;
  public:
    // Initialize GFSK demodulator
    GFSKDemodulator(int samples_per_symbol, float g_mu, float omega_rel_limit);

    // Process IQ samples and return recovered bits
    std::vector<uint8_t> process(const std::vector<std::complex<float>> &IQ);

    // Reset internal state
    void reset();
};

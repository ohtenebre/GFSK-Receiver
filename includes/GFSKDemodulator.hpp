#pragma once

#include <complex>
#include <cstdint>
#include <vector>

// Quadrature demodulator (FM discriminator)
class QuadratureDemod {
  private:
    float d_gain;

    std::complex<float> d_last_sample;

    bool d_first;
  public:
    QuadratureDemod(float gain) : d_gain(gain), d_last_sample(0, 0), d_first(true) {}

    std::vector<float> process(const std::vector<std::complex<float>> &iq);
};

// Symbol timing recovery using Gardner TED
class SymbolSync {
  private:
    float d_omega; // samples per symbol
    float d_mu;    // fractional position

    float d_gain_omega;
    float d_gain_mu;

    float d_last_sample;

    float d_sps;

    float d_max_deviation;

    std::vector<float> buffer;
  public:
    SymbolSync(float sps, float loop_bw, float damping, float ted_gain, float max_dev);

    std::vector<float> process(const std::vector<float> &input);
};

// Convert soft decision values into binary bits
class BinarySlicer {
  public:
    std::vector<uint8_t> process(const std::vector<float> &input)
    {
        std::vector<uint8_t> output;
        output.reserve(input.size());

        for (float x : input)
        {
            // Hard decision: positive value -> 1, negative -> 0
            output.push_back(x >= 0.0f ? 1 : 0);
        }

        return output;
    }
};

#pragma once

#include <complex>
#include <cstdint>
#include <vector>

// Fractional resampler using MMSE interpolation
class FractionalResampler {
  private:
    double d_mu;    // Fractional delay
    double d_ratio; // Input/output sample ratio

    static constexpr int NTAPS = 8;
    static constexpr int NSTEPS = 128;
  public:
    // Initialize resampler with sample rate ratio
    FractionalResampler(double ratio) : d_mu(0.0), d_ratio(ratio) {}

    // Reset interpolation state
    void reset()
    {
        d_mu = 0.0;
    }

    // Resample IQ signal
    std::vector<std::complex<float>> process(const std::vector<std::complex<float>> &input);
  private:
    // Interpolate sample at fractional position
    std::complex<float> interpolate(const std::complex<float> *input, double mu);
};

// Frequency shifter using complex rotation
class FrequencyShifter {
  private:
    float current_phase; // Current oscillator phase
  public:
    FrequencyShifter() : current_phase(0.0f) {}

    // Apply frequency offset correction
    void process(std::vector<std::complex<float>> &IQ, uint32_t sample_rate, float freq_offset);

    // Reset oscillator phase
    void reset()
    {
        current_phase = 0.0f;
    }
};

// Low Pass Filter with blackman window
class FIRFilter {
  private:
    std::vector<float> taps;
    std::vector<float> delay;
  public:
    FIRFilter(std::vector<float> t) : taps(std::move(t)), delay(taps.size(), 0) {}

    std::vector<float> process(const std::vector<float> &input);
};

// Generate low-pass FIR filter coefficients
std::vector<float> generate_lowpass(float gain, float sample_rate, float cutoff, float transition);

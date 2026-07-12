#include "GFSKDemodulator.hpp"
#include "mmse_fir_interpolator.hpp"

// Initialize GFSK demodulator and clock recovery loop
GFSKDemodulator::GFSKDemodulator(int samples_per_symbol, float g_mu, float omega_rel_limit) : last_sample(0.f, 0.f), d_samples_per_symbol(static_cast<float>(samples_per_symbol))
{
    // Quadrature demodulator gain
    const float sensitivity = M_PIf / d_samples_per_symbol;
    d_gain = 1.0f / sensitivity;

    // Clock recovery loop parameters
    const float gain_mu = g_mu;
    const float gain_omega = 0.25f * gain_mu * gain_mu;
    const float loop_bw = -std::log((gain_mu + gain_omega) / (-2.0f) + 1.0f);
    const float damping = 1.0f;
    const float ted_gain = 1.0f;

    // Limit timing correction range
    const float max_deviation = omega_rel_limit * d_samples_per_symbol;

    d_nom_period = d_samples_per_symbol;
    d_min_period = d_nom_period - max_deviation;
    d_max_period = d_nom_period + max_deviation;

    // Loop filter coefficients
    const float zeta_omega_n_T = damping * loop_bw;
    const float k0 = 2.0f / ted_gain;
    const float k1 = std::exp(-zeta_omega_n_T);

    const float sinh_zeta_omega_n_T = std::sinh(zeta_omega_n_T);
    const float cosx_omega_d_T = 1.0f;

    d_alpha = k0 * k1 * sinh_zeta_omega_n_T;
    d_beta = k0 * (1.0f - k1 * (sinh_zeta_omega_n_T + cosx_omega_d_T));

    reset();
}

// Reset demodulator state
void GFSKDemodulator::reset()
{
    last_sample = std::complex<float>(0.f, 0.f);

    d_interp_buf.clear();

    mu = 0.0f;

    d_avg_period = d_samples_per_symbol;
    d_inst_period = d_samples_per_symbol;

    last_rx_symbol = 0.f;
    last_sliced_symbol = 0.f;
}

// Convert symbol value to decision level
static inline float slice(float x)
{
    return x < 0.0f ? -1.0f : 1.0f;
}

// Perform quadrature demodulation and clock recovery
std::vector<uint8_t> GFSKDemodulator::process(const std::vector<std::complex<float>> &IQ)
{
    std::vector<uint8_t> output_bits;
    if (IQ.empty())
        return output_bits;

    // Quadrature FM demodulation
    std::vector<float> fm_demodded;
    fm_demodded.reserve(IQ.size());

    for (const auto &current : IQ)
    {
        std::complex<float> conj_prod = current * std::conj(last_sample);
        float phase_diff = std::arg(conj_prod);

        fm_demodded.push_back(phase_diff * d_gain);

        last_sample = current;
    }

    d_interp_buf.insert(d_interp_buf.end(), fm_demodded.begin(), fm_demodded.end());

    const size_t ntaps = static_cast<size_t>(dsp_detail::NTAPS);
    if (d_interp_buf.size() <= ntaps)
        return output_bits;

    const size_t ni = d_interp_buf.size() - ntaps;

    size_t idx = 0;

    // Mueller and Muller clock recovery
    while (idx < ni)
    {
        float current_rx_symbol = dsp_detail::mmse_interpolate<float>(&d_interp_buf[idx], mu);
        uint8_t bit = (current_rx_symbol > 0.0f) ? 1 : 0;

        float current_sliced_symbol = slice(current_rx_symbol);
        float error = last_sliced_symbol * current_rx_symbol - current_sliced_symbol * last_rx_symbol;

        float avg_period = d_avg_period + d_beta * error;
        if (avg_period > d_max_period)
            avg_period = d_max_period;
        else if (avg_period < d_min_period)
            avg_period = d_min_period;

        float inst_period = avg_period + d_alpha * error;
        if (inst_period <= 0.0f)
            inst_period = avg_period;

        float phase = mu + inst_period;
        float n = std::floor(phase);
        float phase_wrapped = phase - n;
        size_t phase_n = static_cast<size_t>(n);

        if (idx + phase_n >= ni)
            break;

        output_bits.push_back(bit);

        mu = phase_wrapped;
        d_avg_period = avg_period;
        d_inst_period = inst_period;
        last_rx_symbol = current_rx_symbol;
        last_sliced_symbol = current_sliced_symbol;

        idx += phase_n;
    }

    if (idx > 0 && idx <= d_interp_buf.size())
        d_interp_buf.erase(d_interp_buf.begin(), d_interp_buf.begin() + idx);

    return output_bits;
}

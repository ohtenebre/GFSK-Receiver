#include "GFSKDemodulator.hpp"
#include "dsp.hpp"
#include "file_mng.hpp"

#include <vector>

const char *IN_FILENAME = "input.bin";
const char *OUT_IQ_FILENAME = "output_iq.bin";
const char *OUT_BYTES_FILENAME = "output_bytes.bin";

const float SAMPLE_RATE = 2e6;
const float SAMPLE_RATE_AFTER_RESAMPLER = 38400.f;

const float Q_GAIN = 1.27323954;

const float FIR_GAIN = 0.0025f;
const float CUTOFF = 10000.f;
const float TRANSITION = 4000.f;

const float NSPS = 4.f;
const float LOOP_BW = 0.019f;
const float TED_GAIN = 1.5f;
const float MAX_DEV = 1.5f;
const float DAMPING = 0.9f;

const float FREQ_OFFSET = 103.e3f;

int main()
{
    // Initialize DSP blocks
    FrequencyShifter freqshifter;
    FractionalResampler resampler(SAMPLE_RATE / SAMPLE_RATE_AFTER_RESAMPLER);
    QuadratureDemod demod(Q_GAIN);
    auto taps = generate_lowpass(FIR_GAIN, SAMPLE_RATE_AFTER_RESAMPLER, CUTOFF, TRANSITION);
    FIRFilter lpf(taps);
    SymbolSync symsync(NSPS, LOOP_BW, DAMPING, TED_GAIN, MAX_DEV);
    BinarySlicer slicer;

    // Read IQ samples from WAV
    auto IQ = readIQ(IN_FILENAME);
    auto sr = getSampleRate(IN_FILENAME);

    // Correct frequency offset
    freqshifter.process(IQ, sr, FREQ_OFFSET);

    // Resample to symbol rate
    IQ = resampler.process(IQ);

    // Quadrature demodulation
    auto demod_out = demod.process(IQ);

    // FIR
    auto filtered = lpf.process(demod_out);

    // Zero Crossing Symbol Sync
    auto symbols = symsync.process(filtered);

    // Binary Slicer
    auto bits = slicer.process(symbols);

    uint8_t bit_buf = 0;
    uint8_t bit_cnt = 0;

    // Convert bits to bytes
    auto bytes = packBits(bits, bit_buf, bit_cnt);
    if (bit_cnt > 0)
    {
        bit_buf <<= (8 - bit_cnt);
        bytes.push_back(bit_buf);
    }

    writeBytesToBinary(bytes, OUT_BYTES_FILENAME);

    return 0;
}

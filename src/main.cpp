#include "GFSKDemodulator.hpp"
#include "dsp.hpp"
#include "file_mng.hpp"

#include <vector>

const char *IN_FILENAME = "input.bin";
const char *OUT_IQ_FILENAME = "output_iq.bin";
const char *OUT_BYTES_FILENAME = "output_bytes.bin";

const float SAMPLE_RATE = 150000.f;
const float SAMPLE_RATE_AFTER_RESAMPLER = 38400.f;

const int NSPS = 4;
const float G_MU = 0.175f;
const float OMEGA_REL_LIMIT = 0.005f;

const float FREQ_OFFSET = 0.f;

int main()
{
    // Initialize DSP blocks
    FrequencyShifter freqshifter;
    FractionalResampler resampler(SAMPLE_RATE / SAMPLE_RATE_AFTER_RESAMPLER);
    GFSKDemodulator demodulator(NSPS, G_MU, OMEGA_REL_LIMIT);

    // Read IQ samples from WAV
    auto IQ = readIQ(IN_FILENAME);
    auto sr = getSampleRate(IN_FILENAME);

    // Correct frequency offset
    freqshifter.process(IQ, sr, FREQ_OFFSET);

    // Resample to symbol rate
    IQ = resampler.process(IQ);
    writeIQ(IQ, OUT_IQ_FILENAME);

    // GFSK demodulation
    auto bits = demodulator.process(IQ);

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

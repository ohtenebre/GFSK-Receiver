#include "file_mng.hpp"

#include <fstream>
#include <vector>

// Read complex IQ samples from WAV file
std::vector<std::complex<float>> readIQ(const std::string &filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (!file)
        throw std::runtime_error("Cannot open file");

    auto size = file.tellg();
    size -= 44;

    file.seekg(44);

    size_t samples = size / (sizeof(int16_t) * 2);

    std::vector<int16_t> raw(samples * 2);

    file.read(reinterpret_cast<char *>(raw.data()), size);

    std::vector<std::complex<float>> iq;
    iq.reserve(samples);

    for (size_t i = 0; i < samples; i++)
        iq.emplace_back(raw[i * 2] / 32768.0f, raw[i * 2 + 1] / 32768.0f);

    return iq;
}

// Get sample rate from WAV header
uint32_t getSampleRate(const std::string &filename)
{
    std::ifstream file(filename, std::ios::binary);

    if (!file)
        throw std::runtime_error("open error");

    file.seekg(24);

    uint32_t sample_rate;

    file.read(reinterpret_cast<char *>(&sample_rate), sizeof(sample_rate));

    return sample_rate;
}


// Write complex IQ samples to binary file
void writeIQ(const std::vector<std::complex<float>> &IQ, const std::string &filename)
{
    std::ofstream out(filename, std::ios::binary);

    if (!out)
        return;

    out.write(reinterpret_cast<const char *>(IQ.data()), IQ.size() * sizeof(std::complex<float>));
}

// Convert bit stream into bytes
std::vector<uint8_t> packBits(const std::vector<uint8_t> &bits, uint8_t &bit_buffer, uint8_t &bit_count)
{
    std::vector<uint8_t> packed_bytes;
    packed_bytes.reserve((bits.size() + bit_count) / 8);

    for (uint8_t bit : bits)
    {
        uint8_t clean_bit = bit & 1;

        bit_buffer = (bit_buffer << 1) | clean_bit;
        bit_count++;

        if (bit_count == 8)
        {
            packed_bytes.push_back(bit_buffer);
            bit_buffer = 0;
            bit_count = 0;
        }
    }

    return packed_bytes;
}

// Write byte stream to binary file
void writeBytesToBinary(const std::vector<uint8_t> &bytes, const std::string &filename)
{
    if (bytes.empty())
        return;

    std::ofstream out(filename, std::ios::binary);

    if (!out)
        throw std::runtime_error("Не удалось открыть файл для записи байт: " + filename);

    out.write(reinterpret_cast<const char *>(bytes.data()), bytes.size());
}

#pragma once

#include <complex>
#include <cstdint>
#include <string>
#include <vector>

// Get sample rate from WAV file
uint32_t getSampleRate(const std::string &filename);

// Read complex IQ samples from WAV file
std::vector<std::complex<float>> readIQ(const std::string &filename);

// Write IQ samples to binary file
void writeIQ(const std::vector<std::complex<float>> &IQ, const std::string &filename);

// Convert bit stream to byte stream
std::vector<uint8_t> packBits(const std::vector<uint8_t> &bits, uint8_t &bit_buffer, uint8_t &bit_count);

// Write bytes to binary file
void writeBytesToBinary(const std::vector<uint8_t> &bytes, const std::string &filename);

// Write float stream to binary file
void writeFloatsToBinary(const std::vector<float> &samples, const std::string &filename);

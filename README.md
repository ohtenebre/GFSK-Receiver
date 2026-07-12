# GFSK Demodulator

## Features

- IQ WAV file reading
- Frequency shifting
- Fractional resampling
- Quadrature FM demodulation
- Clock recovery using Mueller and Muller algorithm
- Bit extraction and binary output


## Build

Requirements:

- C++ 20
- CMake

Build:
```bash
mkdir build
cd build
cmake ..
make
````

## Run

From the build directory:

```bash
./receiver
```

The program reads an IQ WAV file and produces a binary file with recovered bytes.

## Project Structure

```
.
├── includes
│   ├── GFSKDemodulator.hpp
│   ├── dsp.hpp
│   ├── file_mng.hpp
│   └── mmse_interp_taps.hpp
│
├── src
│   ├── main.cpp
│   ├── GFSKDemodulator.cpp
│   ├── dsp.cpp
│   └── file_mng.cpp
│
└── CMakeLists.txt
```

> Parameters are configured in `main.cpp`.
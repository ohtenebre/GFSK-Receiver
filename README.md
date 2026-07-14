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
├── python
│   └── parse_image.py
│
├── includes
│   ├── GFSKDemodulator.hpp
│   ├── dsp.hpp
│   ├── file_mng.hpp
│   └── mmse_fir_interpolator.hpp
│
├── third_party
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

## Third-party code

The `third_party` directory contains external code used by the project.

`mmse_interp_taps.hpp` contains MMSE FIR interpolator coefficient tables
ported from GNU Radio:

* Source: GNU Radio `gr-filter/include/gnuradio/filter/interpolator_taps.h`
* Copyright: Free Software Foundation, Inc.
* License: GPL-3.0-or-later

The coefficient table is used by the custom MMSE FIR interpolator to match
GNU Radio's interpolation behavior.

## Configuration

Parameters such as sample rate, frequency shift, and demodulation settings
are configured in `main.cpp`.
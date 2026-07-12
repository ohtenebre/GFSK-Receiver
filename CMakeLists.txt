cmake_minimum_required(VERSION 3.16)
project(receiver VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_SOURCES
    src/main.cpp
    src/file_mng.cpp
    src/dsp.cpp
    src/GFSKDemodulator.cpp
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    src/
    includes/
    third_party/
)

find_library(FFTW3F_LIBRARY NAMES fftw3f fftw3f-3)

target_link_libraries(${PROJECT_NAME} PRIVATE
    pthread
    ${FFTW3F_LIBRARY}
)

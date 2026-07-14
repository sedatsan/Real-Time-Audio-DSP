# Real-Time Multi-Threaded Audio DSP Pipeline

A portfolio project demonstrating high-performance C++ concurrency, real-time digital signal processing, and system-level architecture.

## Overview
This application captures live microphone audio using PortAudio, processes it using a multi-threaded pipeline, and evaluates the frequency spectrum using Fast Fourier Transform (FFT).

## Architecture
- **Producer (Audio Capture):** High-priority real-time thread using PortAudio (ALSA on Linux, WASAPI on Windows).
- **Shared Resource:** Thread-safe `ThreadSafeQueue` (Ring Buffer) for lock-free data transfer.
- **Consumer (DSP Engine):** Processes audio chunks applying Hann windowing and FFT.
- **Main Thread:** Orchestrates lifecycle and handles application state.

## Architecture Highlights
- **SPSC Ring Buffer**: A custom lock-free Single-Producer Single-Consumer queue ensuring zero-mutex audio capture.
- **Custom FFT Engine**: Radix-2 Cooley-Tukey algorithm with pre-computed twiddle factors and Hamming windowing.

## Performance Metrics
Extensive Catch2 benchmarks executed on standard CI runners yield the following quantified metrics:

* **SPSC Lock-Free Ring Buffer**
  * **Latency**: ~65 ns per push/pop operation
  * **Throughput**: >15 Million operations per second (preventing any audio thread bottlenecks).

* **1024-Point FFT DSP Engine**
  * **Forward FFT**: ~528 μs
  * **Full Pipeline** (Windowing + FFT + Magnitude Calculation): ~578 μs
  * **Real-Time Speedup**: At a 44.1 kHz sample rate, a 1024-sample block provides ~23.2 ms of headroom. Processing it in ~0.58 ms utilizes only **~2.5%** of the available time window, granting a **~40x speedup** against the real-time deadline limit.

## Build Requirements
- **C++23 Compiler** (GCC 13+ or MSVC 19.38+)
- **CMake 3.15+**
- **vcpkg** (Used in manifest mode to fetch `portaudio` and `raylib`)

## Getting Started

### Windows (MSVC)
```powershell
# Configure CMake using your vcpkg toolchain
cmake -B build -DCMAKE_TOOLCHAIN_FILE="C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake"

# Build the project
cmake --build build --config Release

# Run the visualizer
.\build\Release\RealTimeAudioDSP.exe
```

### Linux / WSL (Ubuntu)
Ensure you have the required X11, OpenGL, and ALSA dependencies installed:
```bash
sudo apt-get install build-essential libasound2-dev libxinerama-dev libxcursor-dev xorg-dev libglu1-mesa-dev pkg-config
```

Then build the project:
```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build

./build/RealTimeAudioDSP
```

# Real-Time Multi-Threaded Audio DSP Pipeline

A portfolio project demonstrating high-performance C++ concurrency, real-time digital signal processing, and system-level architecture.

## Overview
This application captures live microphone audio using PortAudio, processes it using a multi-threaded pipeline, and evaluates the frequency spectrum using Fast Fourier Transform (FFT).

## Architecture
- **Producer (Audio Capture):** High-priority real-time thread using PortAudio (ALSA on Linux, WASAPI on Windows).
- **Shared Resource:** Thread-safe `ThreadSafeQueue` (Ring Buffer) for lock-free data transfer.
- **Consumer (DSP Engine):** Processes audio chunks applying Hann windowing and FFT.
- **Main Thread:** Orchestrates lifecycle and handles application state.

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
